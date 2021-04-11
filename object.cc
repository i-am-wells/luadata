#include "luadata/object.h"
#include "luadata/root_object.h"

namespace luadata {

Object::Object(int own_table_index, RootObject* root, Object* prev)
    : own_table_index_(own_table_index),
      root_(root),
      prev_(prev),  // Note: prev->next is set by RootObject::AddObject()
      next_(nullptr) {}

Object::~Object() {
  root_->RemoveObject(this);
}

int Object::Count() const {
  return luaL_len(lua_state(), own_table_index_);
}

std::unique_ptr<Object> Object::GetObject(const std::string& key) const {
  if (!PushString(key))
    return nullptr;
  return GetObject();
}

std::unique_ptr<Object> Object::GetObject(const char* key) const {
  return GetObject(std::string(key));
}
std::unique_ptr<Object> Object::GetObject(int key) const {
  if (!PushInt(key))
    return nullptr;
  return GetObject();
}
std::unique_ptr<Object> Object::GetObject(double key) const {
  if (!PushDouble(key))
    return nullptr;
  return GetObject();
}
std::unique_ptr<Object> Object::GetObject(bool key) const {
  if (!PushBool(key))
    return nullptr;
  return GetObject();
}

int Object::GetInt(const std::string& key, int default_value) const {
  if (!PushString(key))
    return default_value;
  return GetIntOrDefault(default_value);
}

int Object::GetInt(const char* key, int default_value) const {
  return GetInt(std::string(key), default_value);
}

int Object::GetInt(int key, int default_value) const {
  if (!PushInt(key))
    return default_value;
  return GetIntOrDefault(default_value);
}

int Object::GetInt(double key, int default_value) const {
  if (!PushDouble(key))
    return default_value;
  return GetIntOrDefault(default_value);
}

int Object::GetInt(bool key, int default_value) const {
  if (!PushBool(key))
    return default_value;
  return GetIntOrDefault(default_value);
}

double Object::GetNumber(const std::string& key, double default_value) const {
  if (!PushString(key))
    return default_value;
  return GetDoubleOrDefault(default_value);
}

double Object::GetNumber(const char* key, double default_value) const {
  return GetNumber(std::string(key), default_value);
}

double Object::GetNumber(int key, double default_value) const {
  if (!PushInt(key))
    return default_value;
  return GetDoubleOrDefault(default_value);
}

double Object::GetNumber(double key, double default_value) const {
  if (!PushDouble(key))
    return default_value;
  return GetDoubleOrDefault(default_value);
}

double Object::GetNumber(bool key, double default_value) const {
  if (!PushBool(key))
    return default_value;
  return GetDoubleOrDefault(default_value);
}

bool Object::GetBool(const std::string& key, bool default_value) const {
  if (!PushString(key))
    return default_value;
  return GetBoolOrDefault(default_value);
}

bool Object::GetBool(const char* key, bool default_value) const {
  return GetBool(std::string(key), default_value);
}

bool Object::GetBool(int key, bool default_value) const {
  if (!PushInt(key))
    return default_value;
  return GetBoolOrDefault(default_value);
}

bool Object::GetBool(double key, bool default_value) const {
  if (!PushDouble(key))
    return default_value;
  return GetBoolOrDefault(default_value);
}

bool Object::GetBool(bool key, bool default_value) const {
  if (!PushBool(key))
    return default_value;
  return GetBoolOrDefault(default_value);
}

std::string Object::GetString(const std::string& key,
                              const std::string& default_value) const {
  if (!PushString(key))
    return default_value;
  return GetStringOrDefault(default_value);
}

std::string Object::GetString(const char* key,
                              const std::string& default_value) const {
  return GetString(std::string(key), default_value);
}

std::string Object::GetString(int key, const std::string& default_value) const {
  if (!PushInt(key))
    return default_value;
  return GetStringOrDefault(default_value);
}

std::string Object::GetString(double key,
                              const std::string& default_value) const {
  if (!PushDouble(key))
    return default_value;
  return GetStringOrDefault(default_value);
}

std::string Object::GetString(bool key,
                              const std::string& default_value) const {
  if (!PushBool(key))
    return default_value;
  return GetStringOrDefault(default_value);
}

bool Object::PushString(const std::string& key) const {
  if (!lua_state())
    return false;
  lua_pushstring(lua_state(), key.c_str());
  return true;
}

bool Object::PushInt(int key) const {
  if (!lua_state())
    return false;
  lua_pushinteger(lua_state(), key);
  return true;
}

bool Object::PushDouble(double key) const {
  if (!lua_state())
    return false;
  lua_pushnumber(lua_state(), key);
  return true;
}

bool Object::PushBool(bool key) const {
  if (!lua_state())
    return false;
  lua_pushboolean(lua_state(), key);
  return true;
}

std::unique_ptr<Object> Object::GetObject() const {
  if (lua_gettable(lua_state(), own_table_index_) != LUA_TTABLE) {
    lua_pop(lua_state(), 1);
    return nullptr;
  }

  // Leave the table on the stack and point the new Object at it.
  return root_->AddObject(lua_gettop(lua_state()));
}

int Object::GetIntOrDefault(int default_value) const {
  if (lua_gettable(lua_state(), own_table_index_) != LUA_TNUMBER) {
    lua_pop(lua_state(), 1);
    return default_value;
  }

  int result = lua_tointeger(lua_state(), -1);
  lua_pop(lua_state(), 1);
  return result;
}

double Object::GetDoubleOrDefault(double default_value) const {
  if (lua_gettable(lua_state(), own_table_index_) != LUA_TNUMBER) {
    lua_pop(lua_state(), 1);
    return default_value;
  }

  int result = lua_tonumber(lua_state(), -1);
  lua_pop(lua_state(), 1);
  return result;
}

bool Object::GetBoolOrDefault(bool default_value) const {
  if (lua_gettable(lua_state(), own_table_index_) != LUA_TBOOLEAN) {
    lua_pop(lua_state(), 1);
    return default_value;
  }

  bool result = lua_toboolean(lua_state(), -1);
  lua_pop(lua_state(), 1);
  return result;
}

std::string Object::GetStringOrDefault(const std::string& default_value) const {
  if (lua_gettable(lua_state(), own_table_index_) != LUA_TSTRING) {
    lua_pop(lua_state(), 1);
    return default_value;
  }

  std::string result(lua_tostring(lua_state(), -1));
  lua_pop(lua_state(), 1);
  return result;
}

lua_State* Object::lua_state() const {
  return root_->lua_state_;
}

}  // namespace luadata