#ifndef LUADATA_OBJECT_H_
#define LUADATA_OBJECT_H_

#include <memory>
#include <string>

#include <lua.hpp>

namespace luadata {

class RootObject;

class Object {
 public:
  virtual ~Object();

  int Count() const;
  // TODO iterators?

  std::unique_ptr<Object> GetObject(const std::string& key) const;
  std::unique_ptr<Object> GetObject(const char* key) const;
  std::unique_ptr<Object> GetObject(int key) const;
  std::unique_ptr<Object> GetObject(double key) const;
  std::unique_ptr<Object> GetObject(bool key) const;

  int GetInt(const std::string& key, int default_value = 0) const;
  int GetInt(const char* key, int default_value = 0) const;
  int GetInt(int key, int default_value = 0) const;
  int GetInt(double key, int default_value = 0) const;
  int GetInt(bool key, int default_value = 0) const;

  double GetNumber(const std::string& key, double default_value = 0) const;
  double GetNumber(const char* key, double default_value = 0) const;
  double GetNumber(int key, double default_value = 0) const;
  double GetNumber(double key, double default_value = 0) const;
  double GetNumber(bool key, double default_value = 0) const;

  bool GetBool(const std::string& key, bool default_value = false) const;
  bool GetBool(const char* key, bool default_value = false) const;
  bool GetBool(int key, bool default_value = false) const;
  bool GetBool(double key, bool default_value = false) const;
  bool GetBool(bool key, bool default_value = false) const;

  std::string GetString(const std::string& key,
                        const std::string& default_value = "") const;
  std::string GetString(const char* key,
                        const std::string& default_value = "") const;
  std::string GetString(int key, const std::string& default_value = "") const;
  std::string GetString(double key,
                        const std::string& default_value = "") const;
  std::string GetString(bool key, const std::string& default_value = "") const;

 private:
  Object(int own_table_index, RootObject* root, Object* prev);

  // These all push a value onto the stack.
  bool PushString(const std::string& key) const;
  bool PushInt(int key) const;
  bool PushDouble(double key) const;
  bool PushBool(bool key) const;

  // This one leaves the table on the stack and creates a new Object from it.
  std::unique_ptr<Object> GetObject() const;

  // These all effectively pop a value from the stack.
  int GetIntOrDefault(int default_value) const;
  double GetDoubleOrDefault(double default_value) const;
  bool GetBoolOrDefault(bool default_value) const;
  std::string GetStringOrDefault(const std::string& default_value) const;

  void GrowLuaStackIfNeeded() const;
  lua_State* lua_state() const;

  friend class RootObject;
  int own_table_index_;
  RootObject* root_;
  Object* prev_;
  Object* next_;
};

}  // namespace luadata

#endif  // LUADATA_OBJECT_H_