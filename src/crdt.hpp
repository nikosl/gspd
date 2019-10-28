#pragma once

#include <string>
#include <unordered_map>
namespace crdt {
class GCounter {
public:
  explicit GCounter(const std::string &);
  void increment();
  int value() const;
  bool compare(const GCounter &other);
  bool merge(const GCounter &other);
  std::unordered_map<std::string, int> payload() const;
  void set_payload(std::unordered_map<std::string, int> );

  template<typename Writer>
  void serialize(Writer &writer) const {
    writer.StartObject();
    writer.String("crdt");
    writer.String("gcounter");
    writer.String("id");
    writer.String(my_id_.c_str());
    writer.String("value");
    writer.Int(value());
    writer.String("payload");
    writer.StartArray();
    for (const auto &p:payload_) {
      writer.StartObject();
      writer.String("id");
      writer.String(p.first.c_str());

      writer.String("value");
      writer.Int(p.second);
      writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
  }

private:
  std::unordered_map<std::string, int> payload_;
  std::string my_id_;
};
} // namespace crdt

