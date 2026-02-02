gCollisions = 0

abs = math.abs

for ent(Position) do
    print("Position of entity " .. ecs.entity_id(ent) .. " is " .. vec(ent.Position))
end

system(entity(Position, Velocity))
    entity.Position += entity.Velocity * ecs.delta_time()
end

system(entity(Velocity, Gravity))
    entity.Velocity.y += entity.Gravity.value * ecs.delta_time()
end

system(entity(Position, Velocity, BoxCollider))
    local pos = entity.Position
    local vel = entity.Velocity
    local col = entity.BoxCollider

    if (pos.x - col.sizeX / 2 < 0 and vel.x < 0) then vel.x *= -1 end
    if (pos.y - col.sizeY / 2 < 0 and vel.y < 0) then vel.y *= -1 end
    if (pos.x + col.sizeX / 2 > SCREEN_X and vel.x > 0) then vel.x *= -1 end
    if (pos.y + col.sizeY / 2 > SCREEN_Y and vel.y > 0) then vel.y *= -1 end
end

system(
    alpha(Position, Velocity, BoxCollider, Mass),
    beta(Position, Velocity, BoxCollider, Mass)
)
    if (alpha == beta) then continue end

    local aPos = alpha.Position
    local aVel = alpha.Velocity
    local aCol = alpha.BoxCollider
    local aMass = alpha.Mass.kilos

    local bPos = beta.Position
    local bVel = beta.Velocity
    local bCol = beta.BoxCollider
    local bMass = beta.Mass.kilos

    local delta = bPos - aPos
    local relative = bVel - aVel
    local allowance = vec(aCol.sizeX + bCol.sizeX, aCol.sizeY + bCol.sizeY) / 2

    if (abs(delta.x) > allowance.x or abs(delta.y) > allowance.y) then continue end

    local horizontal = allowance.x - abs(delta.x) < allowance.y - abs(delta.y)
    if (horizontal) then
        local shift = (allowance.x - abs(delta.x)) * delta.x / abs(delta.x) / 2
        aPos.x -= shift
        bPos.x += shift
        if (delta.x * relative.x < 0) then
            aVel.x += relative.x * bMass / (aMass + bMass) * 2
            bVel.x -= relative.x * aMass / (aMass + bMass) * 2
            gCollisions += 1
        end
    else
        local shift = (allowance.y - abs(delta.y)) * delta.y / abs(delta.y) / 2
        aPos.y -= shift
        bPos.y += shift
        if (delta.y * relative.y < 0) then
            aVel.y += relative.y * bMass / (aMass + bMass) * 2
            bVel.y -= relative.y * aMass / (aMass + bMass) * 2
            gCollisions += 1
        end
    end
end
