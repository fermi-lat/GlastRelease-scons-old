# $Header$
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['xmlUtil'])
	if env['PLATFORM'] == 'win32':
	    env.Tool('findPkgPath', package = 'xmlUtil') 
    env.Tool('xmlBaseLib')
    env.Tool('facilitiesLib')
    env.Tool('addLibrary', library = env['xercesLibs'])

def exists(env):
    return 1;
