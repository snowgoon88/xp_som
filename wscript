# waf configuration file .waf --help pour commande par défaut
# Utilisé par CMD dist

APPNAME = 'VisuGL'
VERSION = '0.2'

# root, build dir
top = '.'
out = 'wbuild'

# ********************************************************************** options
def options( opt ):
    opt.load( 'compiler_cxx' )

# **************************************************************** CMD configure
def configure( conf ):
    conf.load( 'compiler_cxx' )
    conf.env['CXXFLAGS'] = ['-D_REENTRANT','-Wall','-fPIC','-g','-std=c++11']
    ##conf.env.INCLUDES_JSON = conf.path.abspath()+'/include'

    ## Require GSL
    conf.check_cfg(package='gsl',
                   uselib_store='GSL',
                   args=['--cflags', '--libs']
    )
    
# ******************************************************************** CMD build
def build( bld ):
    print('→ build from ' + bld.path.abspath())
    ## bld.recurse( 'src' )
    ## bld.recurse( 'xp' )
    bld.recurse( 'test' )
    
