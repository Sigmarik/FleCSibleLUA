-- test.lua
-- Tests basic Lua features without using built-in libs except print.
-- Avoids io, os, table library, coroutine, string library, etc.
-- Uses only language constructs and print for output.

-- Helper: custom concat for two strings (uses .. operator allowed)
local function my_concat(a, b)
  return a .. b
end

-- Arithmetic
local a, b = 7, 3
print("ARITHMETIC")
print("a + b = " .. (a + b))
print("a - b = " .. (a - b))
print("a * b = " .. (a * b))
print("a / b = " .. (a / b))
print("a ^ b = " .. (a ^ b))

-- Tables (basic creation and iteration using numeric indices)
print("TABLES")
local arr = {10, 20, 30}
arr[4] = 40
local i = 1
while true do
  local v = arr[i]
  if v == nil then break end
  print("arr[" .. i .. "] = " .. v)
  i = i + 1
end

-- Dictionary-like table
local dict = { name = "Test", ok = true }
print("dict.name = " .. dict.name)
print("dict.ok = " .. dict.ok)

-- Functions and first-class functions
print("FUNCTIONS")
local function square(x) return x * x end
local f = square
print("square(5) = " .. f(5))
local function map(t, fn)
  local out = {}
  local idx = 1
  while true do
    local v = t[idx]
    if v == nil then break end
    out[idx] = fn(v)
    idx = idx + 1
  end
  return out
end
local squares = map({1,2,3,4}, square)
local j = 1
while true do
  local v = squares[j]
  if v == nil then break end
  print("squares[" .. j .. "] = " .. v)
  j = j + 1
end

-- Simple iterator implemented manually (no coroutine)
print("ITERATOR")
local function simple_range(n)
  local state = { i = 1, n = n }
  local function iter()
    if state.i > state.n then return nil end
    local cur = state.i
    state.i = state.i + 1
    return cur
  end
  return iter
end
local it = simple_range(3)
while true do
  local v = it()
  if v == nil then break end
  print("range value = " .. v)
end

print("All tests completed.")
