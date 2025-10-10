gravity = 9.815

function RotateBoxes(entity, alpha, beta, gama)
    local function overAndOver(alpha, beta)
        return function(x) return x + 1 end, 2, {1, 2, hi = 3}
    end
end


-- function RotateBoxes(entity)
--     entity.Rotation.angle += PI / 4 * World.deltaTime
-- end
