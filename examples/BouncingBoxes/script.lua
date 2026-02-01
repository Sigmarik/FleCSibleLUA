gCollisions = 0

abs = math.abs

alpha = vec(1, 2, 3)
beta = vec(6, 3, 1)
print(alpha + beta * 2)

system(entity(Position, Velocity))
    entity.Position.x += entity.Velocity.x * ecs.delta_time()
    entity.Position.y += entity.Velocity.y * ecs.delta_time()
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

    local deltaX = bPos.x - aPos.x
    local deltaY = bPos.y - aPos.y

    local relativeX = bVel.x - aVel.x
    local relativeY = bVel.y - aVel.y

    local allowanceX = (aCol.sizeX + bCol.sizeX) / 2
    local allowanceY = (aCol.sizeY + bCol.sizeY) / 2

    if (abs(deltaX) > allowanceX or abs(deltaY) > allowanceY) then continue end

    local horizontal = allowanceX - abs(deltaX) < allowanceY - abs(deltaY)
    if (horizontal) then
        local shift = (allowanceX - abs(deltaX)) * deltaX / abs(deltaX) / 2
        aPos.x -= shift
        bPos.x += shift
        if (deltaX * relativeX < 0) then
            aVel.x += relativeX * bMass / (aMass + bMass) * 2
            bVel.x -= relativeX * aMass / (aMass + bMass) * 2
            gCollisions += 1
        end
    else
        local shift = (allowanceY - abs(deltaY)) * deltaY / abs(deltaY) / 2
        aPos.y -= shift
        bPos.y += shift
        if (deltaY * relativeY < 0) then
            aVel.y += relativeY * bMass / (aMass + bMass) * 2
            bVel.y -= relativeY * aMass / (aMass + bMass) * 2
            gCollisions += 1
        end
    end
end
