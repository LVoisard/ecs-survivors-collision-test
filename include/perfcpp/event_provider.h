#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace perf {
class CounterDefinition;
class EventProvider
{
public:
  virtual ~EventProvider() noexcept = default;

  /**
   * Adds events to the counter definition.
   * @param counter_definition Counter definition to add events to.
   */
  virtual void add_events(CounterDefinition& counter_definition) = 0;
};

/**
 * Event provider to add events defined by the perf subsystem.
 */
class PerfSubsystemEventProvider final : public EventProvider
{
public:
  ~PerfSubsystemEventProvider() noexcept override = default;

  /**
   * Adds all general purpose events provided by the perf subsystem.
   * @param counter_definition Counter definition to add perf subsystem GP events to.
   */
  void add_events(CounterDefinition& counter_definition) override;
};

/**
 * Event provider to add virtual time events.
 */
class TimeEventProvider final : public EventProvider
{
public:
  ~TimeEventProvider() noexcept override = default;

  /**
   * Adds virtual time events.
   * @param counter_definition Counter definition to add events to.
   */
  void add_events(CounterDefinition& counter_definition) override;
};

/**
 * Event provider to add metrics.
 */
class MetricEventProvider final : public EventProvider
{
public:
  ~MetricEventProvider() noexcept override = default;

  /**
   * Adds metrics.
   * @param counter_definition Counter definition to add events to.
   */
  void add_events(CounterDefinition& counter_definition) override;
};

/**
 * Event provider to add events defined by the system using the file system.
 */
class SystemSpecificEventProvider final : public EventProvider
{
public:
  ~SystemSpecificEventProvider() noexcept override = default;

  /**
   * Adds events provided by the system.
   * @param counter_definition Counter definition to add events to.
   */
  void add_events(CounterDefinition& counter_definition) override;

  /**
   * Tries to read the type from the provided file.
   *
   * @param path Path of the type file.
   * @return Integer representation of type.
   */
  [[nodiscard]] static std::optional<std::uint32_t> parse_event_file_descriptor_type(std::filesystem::path&& path);

  /**
   * Tries to read the scale from the provided file.
   *
   * @param path Path of the type file.
   * @return Double representation of scale.
   */
  [[nodiscard]] static std::optional<double> parse_event_file_descriptor_scale(std::filesystem::path&& path);

  /**
   * Parses an event file descriptor (typically located somewhere in the /sys/bus/event_source/.. directory).
   * Typically, event file descriptors contain the event code, umask, and some additional data (e.g., ldlat for load
   * latency).
   *
   * @param path Path of the file descriptor.
   * @return A pair of configuration code and (optional) additional information, like load latency. When the descriptor
   * could not be parsed, nullopt will be returned.
   */
  [[nodiscard]] static std::optional<std::pair<std::uint64_t, std::optional<std::uint64_t>>>
  parse_event_file_descriptor_config(const std::filesystem::path& path);

  /**
   * Tries to read a format file and returns the id of the config and the number of bits.
   * Some formats have multiple entries.
   *
   * @param path Path of the format file.
   * @return List of pairs (config id, bits).
   */
  [[nodiscard]] static std::vector<std::pair<std::uint8_t, std::pair<std::uint8_t, std::optional<std::uint8_t>>>>
  parse_event_file_descriptor_format(std::filesystem::path&& path);

  /**
   * Parses an integer (decimal or hex) from a given string.
   *
   * @param value String to parse.
   * @return Integer, if parsable.
   */
  [[nodiscard]] static std::optional<std::uint64_t> parse_integer(const std::string& value);

private:
  /**
   * Reads all events provided by the PMU at the provided path and adds them to the provided counter definition.
   *
   * @param counter_definition Counter definition to add events to.
   * @param pmu_name Name of the PMU.
   * @param path Path in the filesystem, containing event, type, and format files.
   */
  static void add_events(CounterDefinition& counter_definition, const std::string& pmu_name, const std::string& path);

  /**
   * Tries to identify more PMUs by searching "/sys/bus/event_source/devices/" for subfolders with the given pattern.
   * The identified PMUs are appended to the given list of PMUs.
   *
   * @param regex_pattern Pattern the subfolders must match.
   * @param performance_monitoring_units List of PMUs.
   * @return A list of pairs (path, pmu name) where "pmu name" contains "-" instead of "_".
   */
  static void detect_performance_monitoring_units(
    std::string&& regex_pattern,
    std::vector<std::pair<std::string, std::string>>& performance_monitoring_units);
};

/**
 * Event provider to add events defined by AMD's Instruction Based Sampling PMU.
 */
class AMDIbsEventProvider final : public EventProvider
{
public:
  ~AMDIbsEventProvider() noexcept override = default;

  /**
   * Adds events provided by AMD's Instruction Based Sampling PMU.
   * @param counter_definition Counter definition to add events to.
   */
  void add_events(CounterDefinition& counter_definition) override;

private:
  /**
   * Adds events from the IBS Op PMU.
   * @param counter_definition Counter definition to add events to.
   */
  static void add_fetch_events(CounterDefinition& counter_definition);

  /**
   * Adds events from the IBS Op PMU.
   * @param counter_definition Counter definition to add events to.
   */
  static void add_op_events(CounterDefinition& counter_definition);
};

/**
 * Event provider to add events defined by an external (CSV) file.
 */
class CsvFileEventProvider final : public EventProvider
{
public:
  explicit CsvFileEventProvider(const std::string& file_name) noexcept
    : _file_name(file_name)
  {
  }
  ~CsvFileEventProvider() noexcept override = default;

  /**
   * Adds events provided by an external file.
   * @param counter_definition Counter definition to add events to.
   */
  void add_events(CounterDefinition& counter_definition) override;

private:
  const std::string& _file_name;
};

#ifdef PERFCPP_HAS_PROCESSOR_SPECIFIC_EVENTS
/**
 * Event provider to processor-specific events.
 */
class ProcessorSpecificEventProvider final : public EventProvider
{
public:
  ~ProcessorSpecificEventProvider() noexcept override = default;

  /**
   * Adds processor specific events. The implementation will be generated by the CMake file.
   * @param counter_definition Counter definition to add events to.
   */
  void add_events(CounterDefinition& counter_definition) override;
};
#endif

}