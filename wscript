#!/usr/bin/env python
# encoding: utf-8
# waf configuration file .waf --help pour commande par défaut
# Utilisé par CMD dist

## Par défaut ./waf configure va utiliser buildir=wbuild et CXX=g++
## MAIS avec ./waf configure --out=cbuild --check-cxx-compiler=clang++
##      on peut utilise clang :o)
## l'option --color permet de coloriser la sortie du compilo

APPNAME = 'VisuGL'
VERSION = '0.2'

# root, build dir
top = '.'
out = 'wbuild'

opt_flags = '-O3'
debug_flags = '-O0 -g'

# ********************************************************************** options
def options( opt ):
    opt.load( 'compiler_cxx' )

    # option debug
    opt.add_option('--debug', dest='debug', action="store_true", default=False,
                   help='compile with debugging symbols' )

# **************************************************************** CMD configure
def configure( conf ):
    conf.load( 'compiler_cxx' )

    ## To generate 'compile_commands.json' at the root of buildpath
    # to be linked/copied in order to user 'cquery' in emacs through lsp-mode
    # see https://github.com/cquery-project/cquery/wiki/Emacs
    conf.load('clang_compilation_database')
    print( "CXX=",conf.env.CXX)
    
    conf.env['CXXFLAGS'] = ['-D_REENTRANT','-Wall','-fPIC','-std=c++11']
    conf.env.INCLUDES_JSON = conf.path.abspath()+'/include'
    
    ## Require GSL, using wrapper around pkg-config
    conf.check_cfg(package='gsl',
                   uselib_store='GSL',
                   args=['--cflags', '--libs']
    )
    ## Require FTGL, using wraper around pkg-config
    conf.check_cfg(package='ftgl',
                   uselib_store='FTGL',
                   args=['--cflags', '--libs']
    )
    ## Require GAML, using wraper around pkg-onfig
    ## conf.check_cfg(package='gaml',
    ##                uselib_store='GAML',
    ##                args=['--cflags', '--libs']
    ## )
    ## Require OpenGL, using wraper around pkg-onfig
    conf.check_cfg(package='gl',
                   uselib_store='GL',
                   args=['--cflags', '--libs']
    )
    ## Require OpenGL >1.1 (glew), using wraper around pkg-onfig
    conf.check_cfg(package='glew',
                   uselib_store='GLEW',
                   args=['--cflags', '--libs']
    )
    ## Require GLFW3, using wraper around pkg-config
    conf.check_cfg(package='glfw3',
                   uselib_store='GLFW3',
                   args=['--cflags', '--libs']
    )
    ## Require Eigen3, using wrapper around pkg-config
    conf.check_cfg(package='eigen3',
                   uselib_store='EIGEN3',
                   args=['--cflags', '--libs']
    )
    ## Require libpng++, using wrapper around pkg-config
    conf.check_cfg(package='libpng',
                   uselib_store='PNG',
                   args=['--cflags', '--libs']
    )
    ## Require/Check libboost
    conf.env.LIB_BOOST = ['boost_program_options']
    conf.env.LIBPATH_BOOST = ['/usr/lib/x86_64-linux-gnu','/usr/lib/i386-linux-gnu']
    print "Checking for 'BOOST::program_options'"
    conf.find_file( 'lib'+conf.env.LIB_BOOST[0]+'.so', conf.env.LIBPATH_BOOST )

    ## Require/Check pngwriter
    ## WARN: marche avec pngwriter tag: 0.6.0 car les versions suivantes
    ##       s'appelle libPNGwriter, ne fournisse que librairie statique..
    pngwriter_path = '/home/dutech/Projets/pngwriter'
    conf.env.LIB_PNGWRITER = ['pngwriter']
    conf.env.INCLUDES_PNGWRITER  = [pngwriter_path+'/include']
    conf.env.LIBPATH_PNGWRITER = [pngwriter_path+'/lib']
    print "Checking for 'pngwriter'"
    conf.find_file( conf.env.LIB_PNGWRITER[0]+'.h', conf.env.INCLUDES_PNGWRITER )
    conf.find_file( 'lib'+conf.env.LIB_PNGWRITER[0]+'.so', conf.env.LIBPATH_PNGWRITER )

    ## Require AntTweakBar upper in the hierarchy
    print( "Looking for AntTweak" )
    #print( "path="+conf.path.name )
    antnode = conf.path.find_node( '../AntTweakBar' );
    if not antnode :
        from waflib.Errors import ConfigurationError
        raise ConfigurationError( msg='AntTweakBar not fount in '+conf.path.parent.abspath() )
    print( "  AntPath="+antnode.abspath() )
    conf.env.LIB_ANTTWEAKBAR = ['AntTweakBar']
    conf.env.INCLUDES_ANTTWEAKBAR  = [antnode.abspath()+'/include']
    conf.env.LIBPATH_ANTTWEAKBAR = [antnode.abspath()+'/lib']
    conf.find_file( conf.env.LIB_ANTTWEAKBAR[0]+'.h',
                    conf.env.INCLUDES_ANTTWEAKBAR)
    conf.find_file( 'lib'+conf.env.LIB_ANTTWEAKBAR[0]+'.so',
                    conf.env.LIBPATH_ANTTWEAKBAR )
    
# ******************************************************************** CMD build
def build( bld ):
    print('→ build from ' + bld.path.abspath())

    # check debug option
    if bld.options.debug:
        bld.env['CXXFLAGS'] += debug_flags.split(' ')
    else:
        bld.env['CXXFLAGS'] += opt_flags.split(' ')
    print( bld.env['CXXFLAGS'] )
        
    
    bld.recurse( 'xp' )
    bld.recurse( 'test' )
    bld.recurse( 'test/visugl' )
    bld.recurse( 'src/supelec' )
    bld.recurse( 'test/dsom' )	    
    
