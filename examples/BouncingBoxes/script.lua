system(entity(Position, Velocity))
    entity.Position.x += entity.Velocity.x * deltaTime()
    entity.Position.y += entity.Velocity.y * deltaTime()
    end

system(entity(Velocity, Gravity))
    entity.Velocity.y += entity.Gravity.value * deltaTime()
    end

system(entity(Position, Velocity, BoxCollider))
    pos = entity.Position
    vel = entity.Velocity
    col = entity.BoxCollider

    if (pos.x - col.sizeX / 2 < 0 and vel.x < 0) then vel.x *= -1 end
    if (pos.y - col.sizeY / 2 < 0 and vel.y < 0) then vel.y *= -1 end
    if (pos.x + col.sizeX / 2 > 800 and vel.x > 0) then vel.x *= -1 end
    if (pos.y + col.sizeY / 2 > 600 and vel.y > 0) then vel.y *= -1 end
    end
