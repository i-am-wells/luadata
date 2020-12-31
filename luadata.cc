#include <lua.h>
#include <lua.hpp>

#include "luadata.h"

namespace luadata {
namespace {

const char kOkay[] = "all is well";
const char kUnknownLuaError[] = "Unknown Lua error.";
const char kSyntaxError[] = "Lua syntax error.";
const char kFileReadError[] = "Lua reported a file read error.";
const char kOutOfMemory[] = "Lua tried and failed to allocate memory.";
const char kLuaRuntimeError[] = "Lua reported a runtime error.";
const char kEmpty[] =
    "The script didn't return anything. Make sure it looks like 'return {}'.";
const char kTooManyReturnValues[] =
    "The script returned multiple values, but only one is expected.";
const char kReturnValueNotATable[] =
    "Expected a table, but the script returned some other type.";

LuaData::Error ConvertLuaError(int error_from_lua) {
  switch (error_from_lua) {
    case LUA_OK:
      return LuaData::Error::kOkay;
    case LUA_ERRSYNTAX:
      return LuaData::Error::kSyntaxError;
    case LUA_ERRMEM:
      return LuaData::Error::kOutOfMemory;
    case LUA_ERRFILE:
      return LuaData::Error::kFileReadError;
    case LUA_ERRRUN:
      return LuaData::Error::kLuaRuntimeError;
    default:
      return LuaData::Error::kUnknownLuaError;
  }
}

}  // namespace

// static
LuaData::LoadResult LuaData::LoadString(const std::string& lua_text) {
  return LoadInternal(lua_text, /*from_file=*/false);
}

// static
LuaData::LoadResult LuaData::LoadFile(const std::string& file_path) {
  return LoadInternal(file_path, /*from_file=*/true);
}

// static
std::string LuaData::GetErrorString(Error err) {
  switch (err) {
    case Error::kOkay:
      return kOkay;
    case Error::kUnknownLuaError:
      return kUnknownLuaError;
    case Error::kSyntaxError:
      return kSyntaxError;
    case Error::kFileReadError:
      return kFileReadError;
    case Error::kOutOfMemory:
      return kOutOfMemory;
    case Error::kLuaRuntimeError:
      return kLuaRuntimeError;
    case Error::kEmpty:
      return kEmpty;
    case Error::kTooManyReturnValues:
      return kTooManyReturnValues;
    case Error::kReturnValueNotATable:
      return kReturnValueNotATable;
  }
}

// static
LuaData::LoadResult LuaData::LoadInternal(const std::string& source,
                                          bool from_file) {
  lua_State* lua_state = luaL_newstate();
  int load_status;
  if (from_file) {
    load_status = luaL_loadfile(lua_state, source.c_str());
  } else {
    load_status =
        luaL_loadbuffer(lua_state, source.data(), source.size(), "string");
  }

  if (load_status != LUA_OK)
    return {nullptr, ConvertLuaError(load_status)};

  int pcall_status = lua_pcall(lua_state, /*nargs=*/0, /*nresults=*/1,
                               /*message_handler_stack_index=*/0);
  if (pcall_status != LUA_OK)
    return {nullptr, ConvertLuaError(pcall_status)};

  // A well-formed input program returns exactly one Lua table
  int return_count = lua_gettop(lua_state);
  if (return_count == 0)
    return {nullptr, Error::kEmpty};

  if (return_count > 1)
    return {nullptr, Error::kTooManyReturnValues};

  if (!lua_istable(lua_state, 1))
    return {nullptr, Error::kReturnValueNotATable};

  return {std::unique_ptr<LuaData>(new LuaData(lua_state, 1)), Error::kOkay};
}

LuaData::LuaData() : lua_state_(nullptr), own_table_index_(1) {}

LuaData::LuaData(lua_State* lua_state, int own_table_index)
    : lua_state_(lua_state), own_table_index_(own_table_index) {}

LuaData::~LuaData() {
  // Only clean up lua state if we're the root table.
  if (lua_state_ && own_table_index_ == 1)
    lua_close(lua_state_);
}

LuaData LuaData::GetObject(const std::string& key) const {
  if (!PushString(key))
    return LuaData();
  return GetObject();
}

LuaData LuaData::GetObject(int key) const {
  if (!PushInt(key))
    return LuaData();
  return GetObject();
}

LuaData LuaData::GetObject(double key) const {
  if (!PushDouble(key))
    return LuaData();
  return GetObject();
}

LuaData LuaData::GetObject(bool key) const {
  if (!PushBool(key))
    return LuaData();
  return GetObject();
}

int LuaData::GetInt(const std::string& key, int default_value) const {
  if (!PushString(key))
    return default_value;
  return GetIntOrDefault(default_value);
}

int LuaData::GetInt(int key, int default_value) const {
  if (!PushInt(key))
    return default_value;
  return GetIntOrDefault(default_value);
}

int LuaData::GetInt(double key, int default_value) const {
  if (!PushDouble(key))
    return default_value;
  return GetIntOrDefault(default_value);
}

int LuaData::GetInt(bool key, int default_value) const {
  if (!PushBool(key))
    return default_value;
  return GetIntOrDefault(default_value);
}

double LuaData::GetNumber(const std::string& key, double default_value) const {
  if (!PushString(key))
    return default_value;
  return GetDoubleOrDefault(default_value);
}

double LuaData::GetNumber(int key, double default_value) const {
  if (!PushInt(key))
    return default_value;
  return GetDoubleOrDefault(default_value);
}

double LuaData::GetNumber(double key, double default_value) const {
  if (!PushDouble(key))
    return default_value;
  return GetDoubleOrDefault(default_value);
}

double LuaData::GetNumber(bool key, double default_value) const {
  if (!PushBool(key))
    return default_value;
  return GetDoubleOrDefault(default_value);
}

bool LuaData::GetBool(const std::string& key, bool default_value) const {
  if (!PushString(key))
    return default_value;
  return GetBoolOrDefault(default_value);
}

bool LuaData::GetBool(int key, bool default_value) const {
  if (!PushInt(key))
    return default_value;
  return GetBoolOrDefault(default_value);
}

bool LuaData::GetBool(double key, bool default_value) const {
  if (!PushDouble(key))
    return default_value;
  return GetBoolOrDefault(default_value);
}

bool LuaData::GetBool(bool key, bool default_value) const {
  if (!PushBool(key))
    return default_value;
  return GetBoolOrDefault(default_value);
}

std::string LuaData::GetString(const std::string& key,
                               const std::string& default_value) const {
  if (!PushString(key))
    return default_value;
  return GetStringOrDefault(default_value);
}

std::string LuaData::GetString(int key,
                               const std::string& default_value) const {
  if (!PushInt(key))
    return default_value;
  return GetStringOrDefault(default_value);
}

std::string LuaData::GetString(double key,
                               const std::string& default_value) const {
  if (!PushDouble(key))
    return default_value;
  return GetStringOrDefault(default_value);
}

std::string LuaData::GetString(bool key,
                               const std::string& default_value) const {
  if (!PushBool(key))
    return default_value;
  return GetStringOrDefault(default_value);
}

bool LuaData::PushString(const std::string& key) const {
  if (!lua_state_)
    return false;
  lua_pushstring(lua_state_, key.c_str());
  return true;
}

bool LuaData::PushInt(int key) const {
  if (!lua_state_)
    return false;
  lua_pushinteger(lua_state_, key);
  return true;
}

bool LuaData::PushDouble(double key) const {
  if (!lua_state_)
    return false;
  lua_pushnumber(lua_state_, key);
  return true;
}

bool LuaData::PushBool(bool key) const {
  if (!lua_state_)
    return false;
  lua_pushboolean(lua_state_, key);
  return true;
}

LuaData LuaData::GetObject() const {
  if (lua_gettable(lua_state_, own_table_index_) != LUA_TTABLE) {
    lua_pop(lua_state_, 1);
    return LuaData();
  }

  // Leave the table on the stack and point the new LuaData at it.
  return LuaData(lua_state_, lua_gettop(lua_state_));
}

int LuaData::GetIntOrDefault(int default_value) const {
  if (lua_gettable(lua_state_, own_table_index_) != LUA_TNUMBER) {
    lua_pop(lua_state_, 1);
    return default_value;
  }

  int result = lua_tointeger(lua_state_, -1);
  lua_pop(lua_state_, 1);
  return result;
}

double LuaData::GetDoubleOrDefault(double default_value) const {
  if (lua_gettable(lua_state_, own_table_index_) != LUA_TNUMBER) {
    lua_pop(lua_state_, 1);
    return default_value;
  }

  int result = lua_tonumber(lua_state_, -1);
  lua_pop(lua_state_, 1);
  return result;
}

bool LuaData::GetBoolOrDefault(bool default_value) const {
  if (lua_gettable(lua_state_, own_table_index_) != LUA_TBOOLEAN) {
    lua_pop(lua_state_, 1);
    return default_value;
  }

  bool result = lua_toboolean(lua_state_, -1);
  lua_pop(lua_state_, 1);
  return result;
}

std::string LuaData::GetStringOrDefault(
    const std::string& default_value) const {
  if (lua_gettable(lua_state_, own_table_index_) != LUA_TSTRING) {
    lua_pop(lua_state_, 1);
    return default_value;
  }

  std::string result(lua_tostring(lua_state_, -1));
  lua_pop(lua_state_, 1);
  return result;
}

}  // namespace luadata