# $Header$
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['gui'])
	env.Tool('addLibrary', library = ['guisystem'])

def exists(env):
    return 1;
