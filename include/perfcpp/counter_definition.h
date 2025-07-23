#pragma once

#include "counter.h"
#include "event_provider.h"
#include "metric.h"
#include "time_event.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace perf {
/**
 * The CounterDefinition holds names and configurations of events and metrics.
 */
class CounterDefinition
{
public:
  explicit CounterDefinition(std::unique_ptr<EventProvider>&& event_provider = nullptr);
  explicit CounterDefinition(const std::string& config_file);
  explicit CounterDefinition(std::string&& config_file)
    : CounterDefinition(config_file)
  {
  }

  CounterDefinition(CounterDefinition&&) noexcept = default;
  CounterDefinition& operator=(CounterDefinition&&) noexcept = default;

  ~CounterDefinition() = default;

  /**
   * Adds an event with the given name and configuration.
   *
   * See https://github.com/jmuehlig/perf-cpp/blob/dev/docs/counters.md
   *
   * @param pmu_name Name of the PMU.
   * @param event_name Name of the event.
   * @param type Type of the event.
   * @param event_id Id of the event.
   */
  void add(std::string&& pmu_name, std::string&& event_name, const std::uint32_t type, const std::uint64_t event_id)
  {
    add(std::move(pmu_name), std::move(event_name), CounterConfig{ type, event_id });
  }

  /**
   * Adds a RAW event with the given name and configuration.
   *
   * See https://github.com/jmuehlig/perf-cpp/blob/dev/docs/counters.md
   *
   * @param pmu_name Name of the PMU.
   * @param event_name Name of the event.
   * @param event_id Id of the event.
   */
  void add(std::string&& pmu_name, std::string&& event_name, const std::uint64_t event_id)
  {
    add(std::move(pmu_name), std::move(event_name), CounterConfig{ PERF_TYPE_RAW, event_id });
  }

  /**
   * Adds an event with the given name and configuration.
   *
   * See https://github.com/jmuehlig/perf-cpp/blob/dev/docs/counters.md
   *
   * @param pmu_name Name of the PMU.
   * @param event_name Name of the event.
   * @param config Config of the event.
   */
  void add(std::string&& pmu_name, std::string&& event_name, CounterConfig config);

  /**
   * Adds an event with the given name and configuration.
   *
   * See https://github.com/jmuehlig/perf-cpp/blob/dev/docs/counters.md
   *
   * @param name Name of the event.
   * @param type Type of the event.
   * @param event_id Id of the event.
   */
  void add(std::string&& name, const std::uint32_t type, const std::uint64_t event_id)
  {
    add("cpu", std::move(name), type, event_id);
  }

  /**
   * Adds a RAW event with the given name and configuration.
   *
   * See https://github.com/jmuehlig/perf-cpp/blob/dev/docs/counters.md
   *
   * @param name Name of the event.
   * @param event_id Id of the event.
   */
  void add(std::string&& name, const std::uint64_t event_id) { add("cpu", std::move(name), event_id); }

  /**
   * Adds an event with the given name and configuration.
   *
   * See https://github.com/jmuehlig/perf-cpp/blob/dev/docs/counters.md
   *
   * @param config Config of the event.
   */
  void add(std::string&& name, CounterConfig config) { add("cpu", std::move(name), config); }

  /**
   * Adds a metric with the given name.
   *
   * See https://github.com/jmuehlig/perf-cpp/blob/dev/docs/metrics.md
   *
   * @param name Name of the metric.
   * @param metric Metric.
   */
  void add(std::string&& name, std::unique_ptr<Metric>&& metric)
  {
    _metrics.insert(std::make_pair(std::move(name), std::move(metric)));
  }

  /**
   * Adds a metric. The name is provided by the metric.
   *
   * See https://github.com/jmuehlig/perf-cpp/blob/dev/docs/metrics.md
   *
   * @param metric Metric.
   */
  void add(std::unique_ptr<Metric>&& metric) { _metrics.insert(std::make_pair(metric->name(), std::move(metric))); }

  /**
   * Adds a formula metric with the given name and formula.
   *
   * See https://github.com/jmuehlig/perf-cpp/blob/dev/docs/metrics.md#using-formulas
   *
   * @param name Name of the metric.
   * @param formula Expression of the metric.
   */
  void add(std::string&& name, std::string&& formula)
  {
    this->add(std::make_unique<FormulaMetric>(std::move(name), std::move(formula)));
  }

  /**
   * Adds a time event the given name.
   *
   * @param name Name of the time event.
   * @param time_event Time event.
   */
  void add(std::string&& name, std::unique_ptr<TimeEvent>&& time_event)
  {
    _time_events.insert(std::make_pair(std::move(name), std::move(time_event)));
  }

  /**
   * Returns a list of counter configurations with the requested name.
   *
   * @param name Name of the queried counter.
   * @return A list of 3-tuples (name of the PMU, name of the event, event configuration). The list may be empty, when
   * that event does not exist.
   */
  [[nodiscard]] std::vector<std::tuple<std::string_view, std::string_view, CounterConfig>> counter(
    std::string&& name) const noexcept
  {
    return counter(name);
  }

  /**
   * Returns a list of counter configurations with the requested name.
   *
   * @param name Name of the queried counter.
   * @return A list of 3-tuples (name of the PMU, name of the event, event configuration). The list may be empty, when
   * that event does not exist.
   */
  [[nodiscard]] std::vector<std::tuple<std::string_view, std::string_view, CounterConfig>> counter(
    const std::string& name) const noexcept;

  /**
   * Returns a list of counter configurations with the requested name.
   *
   * @param name Name of the queried counter.
   * @return A list of 3-tuples (name of the PMU, name of the event, event configuration). The list may be empty, when
   * that event does not exist.
   */
  [[nodiscard]] std::vector<std::tuple<std::string_view, std::string_view, CounterConfig>> counter(
    const std::string_view name) const noexcept
  {
    return counter(std::string{ name });
  }

  /**
   * Returns the counter configurations with the requested event name for a specified PMU.
   *
   * @param pmu_name Name of the PMU.
   * @param event_name Name of the event.
   * @return Tuple of PMU name, event name, and event configuration.
   */
  [[nodiscard]] std::optional<std::tuple<std::string_view, std::string_view, CounterConfig>> counter(
    const std::string& pmu_name,
    const std::string& event_name) const noexcept;

  /**
   * Returns the counter configurations with the requested event name for a specified PMU.
   *
   * @param pmu_name Name of the PMU.
   * @param event_name Name of the event.
   * @return Tuple of PMU name, event name, and event configuration.
   */
  [[nodiscard]] std::optional<std::tuple<std::string_view, std::string_view, CounterConfig>> counter(
    const std::string_view pmu_name,
    const std::string_view event_name) const noexcept
  {
    return counter(std::string{ pmu_name }, std::string{ event_name });
  }

  /**
   * Returns all events for a given PMU. If the PMU is unknown, the list will be empty.
   *
   * @param pmu_name Name of the PMU.
   * @return List of event pairs (name, configuration).
   */
  [[nodiscard]] std::vector<std::pair<std::string_view, CounterConfig>> pmu(const std::string& pmu_name) const;

  /**
   * Checks if a metric with the given name is registered.
   *
   * @param name Name of the requested query.
   * @return True, if the metric exists.
   */
  [[nodiscard]] bool is_metric(const std::string& name) const noexcept { return _metrics.find(name) != _metrics.end(); }

  /**
   * Checks if a metric with the given name is registered.
   *
   * @param name Name of the requested metric.
   * @return True, if the metric exists.
   */
  [[nodiscard]] bool is_metric(std::string_view name) const noexcept
  {
    return is_metric(std::string{ name.data(), name.size() });
  }

  /**
   * Checks if a specific metric is registered and returns the name and the metric.
   *
   * @param name Name of the queried metric.
   * @return Metric and config, std::nullopt of the metric does not exist.
   */
  [[nodiscard]] std::optional<std::pair<std::string_view, Metric&>> metric(const std::string& name) const noexcept;

  /**
   * Checks if a specific metric is registered and returns the name and the metric.
   *
   * @param name Name of the queried metric.
   * @return Metric and config, std::nullopt of the metric does not exist.
   */
  [[nodiscard]] std::optional<std::pair<std::string_view, Metric&>> metric(std::string&& name) const noexcept
  {
    return metric(name);
  }

  /**
   * Checks if a specific metric is registered and returns the name and the metric.
   *
   * @param name Name of the queried metric.
   * @return Metric and config, std::nullopt of the metric does not exist.
   */
  [[nodiscard]] std::optional<std::pair<std::string_view, Metric&>> metric(const std::string_view name) const noexcept
  {
    return metric(std::string{ name.data(), name.size() });
  }

  /**
   * Checks if a time event with the given name is registered.
   *
   * @param name Name of the requested time event.
   * @return True, if the time event exists.
   */
  [[nodiscard]] bool is_time_event(const std::string& name) const noexcept
  {
    return _time_events.find(name) != _time_events.end();
  }

  /**
   * Checks if a time event with the given name is registered.
   *
   * @param name Name of the requested time event.
   * @return True, if the time event exists.
   */
  [[nodiscard]] bool is_time_event(std::string&& name) const noexcept { return is_time_event(name); }

  /**
   * Checks if a time event with the given name is registered.
   *
   * @param name Name of the requested time event.
   * @return True, if the time event exists.
   */
  [[nodiscard]] bool is_time_event(const std::string_view name) const noexcept
  {
    return is_time_event(std::string{ name.data(), name.size() });
  }

  /**
   * Checks if a specific time event is registered and returns the name and the time event.
   *
   * @param name Name of the queried time event.
   * @return Time event and config, std::nullopt of the time event does not exist.
   */
  [[nodiscard]] std::optional<std::pair<std::string_view, TimeEvent&>> time_event(
    const std::string& name) const noexcept;

  /**
   * Checks if a specific time event is registered and returns the name and the time event.
   *
   * @param name Name of the queried time event.
   * @return Time event and config, std::nullopt of the time event does not exist.
   */
  [[nodiscard]] std::optional<std::pair<std::string_view, TimeEvent&>> time_event(std::string&& name) const noexcept
  {
    return time_event(name);
  }

  /**
   * Checks if a specific time event is registered and returns the name and the time event.
   *
   * @param name Name of the queried time event.
   * @return Time event and config, std::nullopt of the time event does not exist.
   */
  [[nodiscard]] std::optional<std::pair<std::string_view, TimeEvent&>> time_event(
    const std::string_view name) const noexcept
  {
    return time_event(std::string{ name.data(), name.size() });
  }

  /**
   * @return List of names of all available performance monitoring units.
   */
  [[nodiscard]] std::vector<std::string> pmu_names() const
  {
    auto names = std::vector<std::string>{};
    std::transform(_performance_monitoring_unit_events.begin(),
                   _performance_monitoring_unit_events.end(),
                   std::back_inserter(names),
                   [](const auto& config) { return config.first; });
    return names;
  }

  /**
   * Reads and adds counters from the provided CSV file with counter configurations.
   * @param csv_filename CSV file with counter configurations.
   */
  [[deprecated("Adding events from a file after construction will be removed with v0.13. Use the constructor and "
               "provide a file instead.")]] void
  read_counter_configuration(const std::string& csv_filename);

  /**
   * @return A table containing all events, metrics, and virtual time events.
   */
  [[nodiscard]] std::string to_string() const;

private:
  /// List of added counter configurations for different PMUs. Each PMU can have multiple counters; but different PMUs
  /// can have the same counter name with different configurations.
  std::unordered_map<std::string, std::unordered_map<std::string, CounterConfig>> _performance_monitoring_unit_events;

  /// List of added metrics.
  std::unordered_map<std::string, std::unique_ptr<Metric>> _metrics;

  /// List of time events.
  std::unordered_map<std::string, std::unique_ptr<TimeEvent>> _time_events;
};
}