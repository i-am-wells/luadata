#ifndef LUADATA_H_
#define LUADATA_H_

#include <memory>
#include <string>

#include <lua.hpp>

namespace luadata {

class LuaData {
 public:
  enum class Error {
    // All is well
    kOkay,

    // Lower-level Lua loading, parsing and executing errors
    kUnknownLuaError,
    kSyntaxError,
    kFileReadError,
    kOutOfMemory,
    kLuaRuntimeError,

    // Errors about badly-formed data
    kEmpty,
    kTooManyReturnValues,
    kReturnValueNotATable,
  };
  static std::string GetErrorString(Error err);

  struct LoadResult {
    std::unique_ptr<LuaData> data;
    Error error;
  };

  static LoadResult LoadString(const std::string& lua_text);
  static LoadResult LoadFile(const std::string& file_path);

  LuaData();
  ~LuaData();

  int Count() const;
  // TODO iterators?

  LuaData GetObject(const std::string& key) const;
  LuaData GetObject(const char* key) const;
  LuaData GetObject(int key) const;
  LuaData GetObject(double key) const;
  LuaData GetObject(bool key) const;

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
  static LoadResult LoadInternal(const std::string& source, bool from_file);

  LuaData(lua_State* lua_state, int own_table_index);

  // These all push a value onto the stack.
  bool PushString(const std::string& key) const;
  bool PushInt(int key) const;
  bool PushDouble(double key) const;
  bool PushBool(bool key) const;

  // This one leaves the table on the stack and creates a new LuaData from it.
  LuaData GetObject() const;

  // These all effectively pop a value from the stack.
  int GetIntOrDefault(int default_value) const;
  double GetDoubleOrDefault(double default_value) const;
  bool GetBoolOrDefault(bool default_value) const;
  std::string GetStringOrDefault(const std::string& default_value) const;

  lua_State* lua_state_;
  int own_table_index_;
};

}  // namespace luadata

#endif  // LUADATA_H_
