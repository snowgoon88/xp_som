#!/usr/bin/python
# -*- coding: utf-8 -*-

# ##############################################################################
# Given a set of parameters, run each setting nb_try times.
# Try to have n-proc processes running at every moment
# ##############################################################################

import subprocess as sp
import itertools as it
## Progress bar
import progress as pb

## Essaie de lancer
## wbuild/xp/xp-001-pomdp -p data_xp/cheese_maze_0.9_1.json -t data_xp/traj_1000.data -e data_xp/esn_50_1_0.99_0.1.json --regul 10.0 -o data_xp/result_10.data

# ******************************************************************* sub_xp01
def sub_xp01( name_pomdp, name_traj, name_esn, regul, name_output, post=""):
    """
    Params of the xp-001-pomdp.cpp
    - idx : to add at the end of every file name
    """

    args = ['wbuild/xp/xp-001-pomdp']
    args.extend( ['-p', name_pomdp] )
    args.extend( ['-t', name_traj] )
    args.extend( ['-e', name_esn] )
    args.extend( ['--regul', str(regul)] )
    args.extend( ['-o', name_output+post] )

    return sp.Popen( args )
# ********************************************************************* repeat
def repeat( name_pomdp, name_traj, name_esn, regul, name_output,
            nb_repeat, idx_start=0, nb_max=2 ):
    """
    Launch nb_repeat copies of function fun, with parameters param in parallel
    At most nb_max process at a time.

    TODO: take nb_max into account
    """
    # static arguments
    args = ['wbuild/xp/xp-001-pomdp']
    args.extend( ['-p', name_pomdp] )
    args.extend( ['-t', name_traj] )
    args.extend( ['-e', name_esn] )
    args.extend( ['--regul', str(regul)] )
    # repeat
    for idx in range(nb_repeat):
        tmp_args = list(args)
        post = "_{0:03d}".format( idx_start+idx ) 
        tmp_args.extend( ['-o', name_output+post] )
        ## print "  →",tmp_args
        sp.Popen( tmp_args ).wait()
# ************************************************************************* xp
def xp():
    """
    Lance XP en faisant une combinaison de tous les paramètres
    """
    base_args = ["wbuild/xp/xp-001-pomdp",
                 "-p", "data_xp/cheese_maze_0.9_1.json"]
    l_traj_size = [100, 1000, 2000, 10000]
    l_esn_size = [50, 100, 200, 500]
    l_regul = [0.01, 0.1, 1, 10]
    l_leak = [0.1, 0.5, 0.9]
    nb_repeat  = 10       ## nb laucnh of each configuration
    nb_start   = 0       ## start numbering files with
    generate   = True    ## need to generate traj,esn

    if generate:
        ## Pour chaque longueur de trajectoire
        print "__TRAJ","  traj_size="+str(l_traj_size)
        for traj_size in l_traj_size:
            traj_name = "data_xp/traj_"+str(traj_size)
            traj_args = [ "--gene_traj", traj_name,
                          "--traj_length", str(traj_size) ]
            ## generate traj
            cmd_args = list( base_args)
            cmd_args.extend( traj_args )
            ## print "→",cmd_args
            sp.Popen( cmd_args ).wait()

        ## Pour chaque esn (regul x leak)
        print "__ESN","  esn_size="+str(l_esn_size)+" leak="+str(l_leak)
        nb_combination = len(l_esn_size)*len(l_leak)
        pbar = pb.ProgressBar(maxval=nb_combination,
                              widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
        id_xp = 0
        for esn_size,leak_rate in it.product(l_esn_size, l_leak):
            esn_name = "data_xp/esn_"+str(esn_size)+"_1_0.99_"+str(leak_rate)
            esn_args = [ "--gene_esn", esn_name,
                         "--res_size", str(esn_size),
                         "--res_leak", str(leak_rate)]
            ## Generate ESN
            cmd_args = list( base_args)
            cmd_args.extend( esn_args )
            ## print "→",cmd_args
            sp.Popen( cmd_args ).wait()
            id_xp +=1
            pbar.update( id_xp )
        pbar.finish()

    ## Apprentissage pour toutes les combinaisons
    print "__LEARN","  "+str(nb_repeat)+" x regul="+str(l_regul)
    nb_combination = len(l_traj_size)*len(l_esn_size)*len(l_leak)*len(l_regul)
    pbar = pb.ProgressBar(maxval=nb_combination,
                          widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
    id_xp = 0
    for traj_size,esn_size,leak,regul in it.product( l_traj_size, l_esn_size, l_leak, l_regul ):
        traj_name = "data_xp/traj_"+str(traj_size)+".data"
        esn_name = "data_xp/esn_"+str(esn_size)+"_1_0.99_"+str(leak)+".json"
        repeat( name_pomdp = "data_xp/cheese_maze_0.9_1.json",
                name_traj=    traj_name,
                name_esn=     esn_name,
                regul =       regul,
                name_output = "data_xp/result_"+str(traj_size)+"_"+str(esn_size)+"_"+str(leak)+"_"+str(regul)+".data",
                nb_repeat = nb_repeat,
                idx_start = nb_start
        )
        id_xp += 1
        pbar.update( id_xp )
    pbar.finish()

# ************************************************************************* MAIN
if __name__ == '__main__':
    print "__RUN "
    # sub_xp01( name_pomdp=   "data_xp/cheese_maze_0.9_1.json",
    #           name_traj=    "data_xp/traj_1000.data",
    #           name_esn=     "data_xp/esn_50_1_0.99_0.1.json",
    #           regul=        10.0,
    #           name_output = "data_xp/result_10.data" ).wait()
    # repeat( name_pomdp=   "data_xp/cheese_maze_0.9_1.json",
    #         name_traj=    "data_xp/traj_1000.data",
    #         name_esn=     "data_xp/esn_50_1_0.99_0.1.json",
    #         regul=        10.0,
    #         name_output = "data_xp/result_10.data",
    #         nb_repeat=5
    # )
    xp()
    print "__END"

    
    
    

    
