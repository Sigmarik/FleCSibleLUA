function overAndOver(alpha, beta)
    return function(x) return x + 1 end
end

function RotateBoxes(entity, alpha, beta, gama)
    for x, y, z in overAndOver(1, 10) do
        something()
    end
end


-- function RotateBoxes(entity)
--     entity.Rotation.angle += PI / 4 * World.deltaTime
-- end
