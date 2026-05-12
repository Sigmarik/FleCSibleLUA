system (entity(Velocity, Position))
    entity.Position += entity.Velocity * ecs.delta_time()
end
