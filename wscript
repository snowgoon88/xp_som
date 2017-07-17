#!/usr/bin/env python
# encoding: utf-8
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

# ******************************************************************** CMD build
def build( bld ):
    print('→ build from ' + bld.path.abspath())
    ## bld.recurse( 'src' )
    bld.recurse( 'xp' )
    bld.recurse( 'test' )
    bld.recurse( 'src/supelec' )
    bld.recurse( 'test/dsom' )	    
    
