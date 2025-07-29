def can_build(env, platform):
    env.module_add_dependencies("gsg", ["flecs"], True)
    return True


def configure(env):
    pass
