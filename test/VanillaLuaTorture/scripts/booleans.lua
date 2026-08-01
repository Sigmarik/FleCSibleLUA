do
    local t = true
    local f = false
    if t and not f then print("PASS: boolean basic") else print("FAIL: boolean basic") end

    -- short-circuit evaluation
    local side = 0
    local function set_side() side = 1 return true end
    local r = false and set_side()   -- should not call set_side
    if side == 0 then print("PASS: short-circuit and") else print("FAIL: short-circuit and") end
    local r2 = true or set_side()    -- should not call set_side
    if side == 0 then print("PASS: short-circuit or") else print("FAIL: short-circuit or") end

    -- logical operators return actual values
    local v1 = nil or "default"
    local v2 = "exists" or "default"
    if v1 == "default" and v2 == "exists" then
        print("PASS: logical value selection")
    else
        print("FAIL: logical value selection")
    end
end