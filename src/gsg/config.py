def can_build(env, platform):
    env.module_add_dependencies("gsg", ["ecs"], True)
    return True


def configure(env):
    pass
