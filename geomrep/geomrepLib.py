#$Header$
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['geomrep'])
    env.Tool('geometryLib')
    env.Tool('guiLib')
def exists(env):
    return 1;
