#!/usr/bin/python
# -*- coding: utf-8 -*-

# ##############################################################################
# launch several run of the same xp configuration
# ##############################################################################

import subprocess as sp
import itertools as it
## Progress bar
import progress as pb

## Essaie de lancer
## wbuild/xp/xp-003-hmm -m tmp_hmm.json -t tmp_traj.json -e tmp_esn.json -l 20 -n tmp_noise.data -g --regul 0.01 -l 15 

# ******************************************************************* sub_xp03
def sub_xp03( name_hmm, name_traj, name_esn, regul, name_noise, name_output, post=""):
    """
    Params of the xp-001-pomdp.cpp
    - idx : to add at the end of every file name
    """

    args = ['wbuild/xp/xp-003-hmm']
    args.extend( ['-m', name_hmm] )
    args.extend( ['-t', name_traj] )
    args.extend( ['-e', name_esn] )
    args.extend( ['--regul', str(regul)] )
    if( name_noise ):
        args.extend( ['-n', name_noise] )
    args.extend( ['-o', name_output+post] )

    return sp.Popen( args )
# ********************************************************************* repeat
def repeat( name_hmm, name_traj, name_esn,
            regul, test_length, name_noise,
            name_output, name_save,
            nb_repeat, idx_start=0, nb_max=2 ):
    """
    Launch nb_repeat copies of function fun, with parameters param in parallel
    At most nb_max process at a time.

    TODO: take nb_max into account
    """
    # static arguments
    args = ['wbuild/xp/xp-003-hmm']
    args.extend( ['-m', name_hmm] )
    args.extend( ['-t', name_traj] )
    args.extend( ['-e', name_esn] )
    args.extend( ['--regul', str(regul)] )
    args.extend( ['-l', str(test_length)] )
    if( name_noise ):
        args.extend( ['-n', name_noise] )
    if( name_save ):
        args.extend( ['-s', name_save] )
    # repeat
    for idx in range(nb_repeat):
        tmp_args = list(args)
        post = "_{0:03d}".format( idx_start+idx ) 
        tmp_args.extend( ['-o', name_output+post] )
        # print "  →",tmp_args
        sp.Popen( tmp_args ).wait()
# ************************************************************************* xp
def xp():
    """
    Lance XP en faisant une combinaison de tous les paramètres
    wbuild/xp/xp-003-hmm -m tmp_hmm.json -t tmp_traj.json -e tmp_esn.json -l 20 -n tmp_noise.data -g --regul 0
    """
    base_args = ["wbuild/xp/xp-003-hmm"]
    # l_traj_size = [100, 1000, 2000, 10000]
    # l_esn_size = [50, 100, 200, 500]
    # l_regul = [0.01, 0.1, 1, 10]
    # l_leak = [0.1, 0.5, 0.9]

    # l_traj_size = [500]
    # l_esn_size = [10,50]
    # l_regul = [0.01, 0.1]
    # l_leak = [0.1,0.5]
    # l_test_length = [10]

    l_hmm = ['! .05 ABCDEFEDCB']
    l_hmm_names = ['p05ABCDEFEDCB']
    l_traj_size = [100,500,1000,2000]
    l_esn_size = [10,20]
    l_leak = [0.1,0.5,0.9]
    l_forward = [ True, False ]
    l_noise_length = [0,500]
    l_regul = [0.01, 0.1,1.0,10.0]
    l_test_length = [50]
    
    nb_traj    = 2       ## how many instances of each traj config
    nb_esn     = 5       ## how many instances of each esn config
    nb_noise   = 2
    nb_repeat  = 1       ## no need to repeat : deterministic learning
    nb_start   = 0       ## start numbering files with
    generate_hmm  = True     ## need to generate hmm
    generate_traj = True     ## need to generate traj
    generate_esn  = False     ## need to generate esn
    generate_noise= False     ## need to generate oise
    learn         = True     ## learn
    save_learned  = True     ## save learned ESN 

    if generate_hmm:
        ## Pour chaque expression
        print "__HMM","  ="+str(l_hmm)
        pbar = pb.ProgressBar(maxval=len(l_hmm),
                              widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
        id_hmm = 0
        for hmm_exp,hmm_n in zip(l_hmm,l_hmm_names):
            hmm_name = "data_hmm/hmm_"+hmm_n+".json"
            hmm_args = [ "--create_hmm", hmm_exp,
                         "--save_hmm", hmm_name ]
            ## generate traj
            cmd_args = list( base_args)
            cmd_args.extend( hmm_args )
            ## print "→",cmd_args
            sp.Popen( cmd_args ).wait()
            id_hmm +=1
            pbar.update( id_hmm )
        pbar.finish()
    
    if generate_traj:
        ## Pour chaque longueur de trajectoire
        print "__TRAJ","  "+str(nb_traj)+"x traj_size="+str(l_traj_size)
        nb_combination = nb_traj * len(l_traj_size) * len(l_hmm)
        pbar = pb.ProgressBar(maxval=nb_combination,
                              widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
        id_xp = 0
        for hmm_n, traj_size, id_traj in it.product(l_hmm_names, l_traj_size, range(nb_traj)):

            hmm_name = "data_hmm/hmm_"+hmm_n+".json"
            traj_name = "data_hmm/traj_"+hmm_n+"_"+str(traj_size)+"_n{0:03d}".format( id_traj )+".data"
            traj_args = [ "-m", hmm_name,
                          "--length_traj", str(traj_size),
                          "--save_traj", traj_name ]

            ## generate traj
            cmd_args = list( base_args)
            cmd_args.extend( traj_args )
            ## print "→",cmd_args
            sp.Popen( cmd_args ).wait()
            id_xp +=1
            pbar.update( id_xp )
        pbar.finish()

    if generate_esn:
        ## Pour chaque esn (nb_esn x [esn_size x leak x forward]
        print "__ESN","  "+str(nb_esn)+"x esn_size="+str(l_esn_size)+" leak="+str(l_leak)+ " forward="+str(l_forward)
        nb_combination = nb_esn * len(l_esn_size)*len(l_leak)*len(l_forward)
        pbar = pb.ProgressBar(maxval=nb_combination,
                              widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
        id_xp = 0
        for esn_size,leak_rate,fg_forward,id_esn in it.product(l_esn_size, l_leak, l_forward, range(nb_esn)):
            esn_name = "data_hmm/esn_"+str(esn_size)+"_1_0.99_"+str(leak_rate)
            if( fg_forward ):
                esn_name += "_for"
            esn_name += "_n{0:03d}".format( id_esn )+".json"
            esn_args = [ "--res_size", str(esn_size),
                         "--res_leak", str(leak_rate),
                         "--save_esn", esn_name ]
            if( fg_forward ):
                esn_args.append( "--res_forward" )
            ## Generate ESN
            cmd_args = list( base_args)
            cmd_args.extend( esn_args )
            ## print "→",cmd_args
            sp.Popen( cmd_args ).wait()
            id_xp +=1
            pbar.update( id_xp )
        pbar.finish()

    if generate_noise:
        ## Pour chaque longueur de noide
        print "__NOISE","  "+str(nb_noise)+"x noise_size="+str(l_noise_length)
        nb_combination = nb_noise * len(l_noise_length)
        pbar = pb.ProgressBar(maxval=nb_combination,
                              widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
        id_xp = 0
        for noise_size, id_noise in it.product(l_noise_length, range(nb_noise)):
            noise_name = "data_hmm/noise_"+str(noise_size)+"_n{0:03d}".format( id_noise )
            noise_args = [ "--noise_length", str(noise_size),
                           "--save_noise", noise_name ]

            ## generate traj
            cmd_args = list( base_args)
            cmd_args.extend( noise_args )
            ## print "→",cmd_args
            sp.Popen( cmd_args ).wait()
            id_xp +=1
            pbar.update( id_xp )
        pbar.finish()

    if learn:
        # ## Apprentissage pour toutes les combinaisons
        nb_combination = len(l_hmm_names)*len(l_test_length)*len(l_traj_size)*len(l_esn_size)*len(l_leak)*len(l_forward)*len(l_regul)*len(l_noise_length)
        print "__LEARN","  "+str(nb_esn*nb_traj*nb_noise)+" x nb_config="+str(nb_combination)
        pbar = pb.ProgressBar(maxval=nb_combination,
                              widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
        id_xp = 0
        
        for hmm_expr,traj_size,esn_size,leak,fg_forward,noise_length,regul,length_test in it.product( l_hmm_names, l_traj_size, l_esn_size, l_leak, l_forward, l_noise_length, l_regul, l_test_length ):
            for id_esn,id_traj,id_noise in it.product( range(nb_esn), range(nb_traj), range(nb_noise)):
                hmm_name = "data_hmm/hmm_"+hmm_expr+".json"
                traj_name = "data_hmm/traj_"+hmm_expr+"_"+str(traj_size)+"_n{0:03d}".format( id_traj )+".data"
                esn_name = "data_hmm/esn_"+str(esn_size)+"_1_0.99_"+str(leak)
                if( fg_forward ):
                    esn_name += "_for"
                esn_name += "_n{0:03d}".format( id_esn )+".json"
                noise_name = "data_hmm/noise_"+str(noise_length)+"_n{0:03d}".format( id_noise )+".data"
                
                ## allow saving ESNs
                output_name = hmm_expr+"_"+str(traj_size)+"_"+str(esn_size)+"_"+str(leak)+"_"+str(regul)
                if( fg_forward ):
                    output_name += "_for"
                output_name += "_e{0:03d}".format(id_esn)+"_t{0:03d}".format(id_traj)+"_"+str(noise_length)+"_n{0:03d}".format(id_noise)+"_l"+str(length_test)
                save_name = None
                if( save_learned ):
                    save_name = "data_hmm/New/savedesn_"+output_name+".json"
                output_name = "data_hmm/New/result_"+output_name+".data"
                ## Run XP
                repeat( name_hmm = hmm_name,
                        name_traj=    traj_name,
                        name_esn=     esn_name,
                        regul =       regul,
                        test_length = length_test,
                        name_noise = noise_name,
                        name_output = output_name,
                        name_save = save_name,
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

    
    
    

    
