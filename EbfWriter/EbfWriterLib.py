#$Header$
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['EbfWriter'])
    env.Tool('EventLib')
    env.Tool('guiLib')
    env.Tool('identsLib')
    env.Tool('AcdUtilLib')
    env.Tool('TriggerLib')
    env.Tool('TkrDigiLib')
    env.Tool('astroLib')
def exists(env):
    return 1;
