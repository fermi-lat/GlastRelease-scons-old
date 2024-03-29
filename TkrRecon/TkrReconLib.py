# $Header$
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['TkrRecon'])
        if env['PLATFORM']=='win32' and env.get('CONTAINERNAME','')=='GlastRelease':
	    env.Tool('findPkgPath', package = 'TkrRecon') 
    env.Tool('CalUtilLib')
    #env.Tool('CalibDataLib')
    env.Tool('GlastSvcLib')
    env.Tool('guiLib')
    env.Tool('EventLib')
    env.Tool('TkrUtilLib')
    env.Tool('LdfEventLib')
    env.Tool('geometryLib')
    #env.Tool('RootIoLib')
    env.Tool('addLibrary', library = env['gaudiLibs'])
    env.Tool('addLibrary', library = env['clhepLibs'])
    if env['PLATFORM']=='win32' and env.get('CONTAINERNAME','')=='GlastRelease':
        env.Tool('findPkgPath', package = 'GuiSvc') 
def exists(env):
    return 1;
