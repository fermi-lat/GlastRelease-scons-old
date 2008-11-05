# $Header$
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['calibGenCAL'])
    env.Tool('addLibrary', library = env['rootLibs'])
    env.Tool('addLibrary', library = env['rootGuiLibs'])
    env.Tool('addLibrary', library = env['minuitLibs'])
    env.Tool('CalUtilLib')
    env.Tool('calibUtilLib')
    env.Tool('gcrSelectRootDataLib')
    env.Tool('addLibrary', library = env['clhepLibs'])
    env.Tool('reconRootDataLib')
    env.Tool('digiRootDataLib')
    env.Tool('mcRootDataLib')
    env.Tool('addLibrary', library = env['pythonLibs'])

def exists(env):
    return 1;
