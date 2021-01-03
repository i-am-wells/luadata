// Unit tests for LuaData.
// Compile and run:
// ```
//     clang++ -O0 -g -std=c++17 test.cc luadata.cc -o test -llua
//     ./test
// ```

#include <cassert>
#include <iostream>

#include "luadata.h"

namespace {

using luadata::LuaData;

void TestLoadString() {
  auto [object, error] = LuaData::LoadString("`");
  assert(!object);
  assert(error == LuaData::Error::kSyntaxError);

  auto [object1, error1] = LuaData::LoadString("return 1");
  assert(!object1);
  assert(error1 == LuaData::Error::kReturnValueNotATable);

  auto [object2, error2] = LuaData::LoadString("");
  assert(!object2);
  // assert(error2 == LuaData::Error::kEmpty);

  auto [object3, error3] = LuaData::LoadString("return {}, {}");
  assert(object3.get());

  auto [object4, error4] = LuaData::LoadString("return {}");
  assert(object4.get());
  assert(error4 == LuaData::Error::kOkay);
}

void TestGetData() {
  auto [object, error] = LuaData::LoadString(
      "return {"
      "somestring = 1,"
      "[42] = 2,"
      "[true] = 3,"
      "[3.14] = 4,"
      "}");

  assert(object.get());

  assert(1 == object->GetInt("somestring"));
  assert(2 == object->GetInt(42));
  assert(3 == object->GetInt(true));
  assert(4 == object->GetInt(3.14));
}

void TestGetObject() {
  auto [object, error] = LuaData::LoadString(
      "return {"
      "a = {x = 'some string'},"
      "b = {y = 100},"
      "}");

  assert(object.get());

  LuaData object_a = object->GetObject("a");
  assert(std::string("some string") == object_a.GetString("x"));
}

}  // namespace

int main(int argc, char** argv) {
  TestLoadString();
  TestGetData();
  TestGetObject();
  std::cerr << "Ran all tests.\n";
  return 0;
}
