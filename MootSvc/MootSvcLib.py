# $Header$
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['MootSvc'])

    env.Tool('addLibrary', library = env['gaudiLibs'])
    env.Tool('facilitiesLib')
    env.Tool('xmlBaseLib')
    env.Tool('EventLib')
    env.Tool('LdfEventLib')
    env.Tool('mootCoreLib')
def exists(env):
    return 1;
