do
    local x = 1
    if true then
        local x = 2
        if x == 2 then print("PASS: local scope inner") else print("FAIL: local scope inner") end
    end
    if x == 1 then print("PASS: local scope outer") else print("FAIL: local scope outer") end

    -- global variable (not local)
    global_test = 99
    if global_test == 99 then print("PASS: global variable") else print("FAIL: global variable") end
end