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
def repeat( name_hmm, name_traj, name_rdsom,
            ela, ela_rec, eps, sig_i, sig_r, beta,
            learn_length,
            name_output, queue_size, period_save,
            nb_repeat, idx_start=0, nb_max=2,
            testing=False, nb_test=1
):
    """
    Launch nb_repeat copies of function fun, with parameters param in parallel
    At most nb_max process at a time.

    TODO: take nb_max into account
    """
    # static arguments
    args = ['wbuild/xp/xp-004-rdsom']
    args.extend( ['-m', name_hmm] )
    args.extend( ['-t', name_traj] )
    args.extend( ['-d', name_rdsom] )
    args.extend( ['--dsom_beta', str(beta)] )
    args.extend( ['--dsom_ela', str(ela)] )
    args.extend( ['--dsom_ela_rec', str(ela_rec)] )
    args.extend( ['--dsom_eps', str(eps)] )
    args.extend( ['--dsom_sig_i', str(sig_i)] )
    args.extend( ['--dsom_sig_r', str(sig_r)] )
    if not testing:
        args.extend( ['--learn_length', str(learn_length)] )
    args.extend( ['--queue_size', str(queue_size)] )
    if not testing:
        args.extend( ['--period_save', str(period_save)] )
    
    # repeat
    for idx in range(nb_repeat):
        tmp_args = list(args)
        post = "_{0:03d}".format( idx_start+idx ) 
        tmp_args.extend( ['--save_result', name_output+post] )
        if testing:
            tmp_args.extend( ['--testing'l])
            tmp_args.extend( ['--nb_test', str(nb_test)] )
        print "  →",tmp_args
        sp.Popen( tmp_args ).wait()
# ************************************************************************* xp
def xp():
    """
    Lance XP en faisant une combinaison de tous les paramètres
    wbuild/xp/xp-004-rdsom -t data_rdsom/traj_p05AAAAAF_6000.data -d data_rdsom/rdsom_50.json -g --queue_size 10 --dsom_ela 1.0 --dsom_sig_r 0.1 --dsom_sig_i 0.1 --dsom_beta 0.05 --dsom_ela_rec 0.01 --dsom_eps 0.25
    """
    base_args_hmm = ["wbuild/xp/xp-003-hmm"]
    base_args = ["wbuild/xp/xp-004-rdsom"]
    
    l_hmm = ['! .05 AAAAAF']
    l_hmm_names = ['p05AAAAAF']
    l_traj_size = [6000]
    l_hmm_names_test = ['p05AAAAAF']
    l_traj_size_test = [6000]
    s_nb_test = 2
    l_rdsom_size = [100]
    l_ela = [1.0]
    l_ela_rec = [0.01]
    l_eps = [0.25]
    l_sig_r = [0.1]
    l_sig_i = [0.1]
    l_beta = [0.05]
    l_learn_length = [10000]
    s_period = 1000
    s_queue_size = 10
    
    nb_traj    = 2       ## how many instances of each traj config
    nb_traj_test = 2
    nb_rdsom     = 2       ## how many instances of each esn config
    nb_repeat  = 1       ## no need to repeat : deterministic learning
    nb_start   = 0       ## start numbering files with
    generate_hmm  = False     ## need to generate hmm
    generate_traj = False     ## need to generate traj
    generate_rdsom  = False     ## need to generate esn
    learn         = False    ## learn
    fg_test       = True    ## testing

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
            cmd_args = list( base_args_hmm )
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
            cmd_args = list( base_args_hmm )
            cmd_args.extend( traj_args )
            ## print "→",cmd_args
            sp.Popen( cmd_args ).wait()
            id_xp +=1
            pbar.update( id_xp )
        pbar.finish()

    if generate_rdsom:
        ## Pour chaque rdsom (nb_rdsom x [rdsom_size]
        print "__RDSOM","  "+str(nb_rdsom)+"x rdsom_size="+str(l_rdsom_size)
        nb_combination = nb_rdsom * len(l_rdsom_size)
        pbar = pb.ProgressBar(maxval=nb_combination,
                              widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
        id_xp = 0
        for rdsom_size,id_rdsom in it.product(l_rdsom_size, range(nb_rdsom)):
            rdsom_name = "data_rdsom/rdsom_"+str(rdsom_size)
            rdsom_name += "_n{0:03d}".format( id_rdsom )+".json"
            rdsom_args = [ "--rdsom_size", str(rdsom_size),
                           "--save_rdsom", rdsom_name ]
            ## Generate RDSOM
            cmd_args = list( base_args)
            cmd_args.extend( rdsom_args )
            ## print "→",cmd_args
            sp.Popen( cmd_args ).wait()
            id_xp +=1
            pbar.update( id_xp )
        pbar.finish()


    if learn:
        # ## Apprentissage pour toutes les combinaisons
        nb_combination = len(l_hmm_names)*len(l_traj_size)*len(l_rdsom_size)*len(l_ela)*len(l_ela_rec)*len(l_eps)*len(l_sig_i)*len(l_sig_r)*len(l_beta)*len(l_learn_length)
        print "__LEARN","  "+str(nb_rdsom*nb_traj)+" x nb_config="+str(nb_combination)
        pbar = pb.ProgressBar(maxval=nb_combination,
                              widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
        id_xp = 0
        
        for hmm_expr,traj_size,rdsom_size,ela,ela_rec,eps,sig_i,sig_r,beta,learn_length in it.product( l_hmm_names, l_traj_size, l_rdsom_size, l_ela, l_ela_rec, l_eps, l_sig_i, l_sig_r, l_beta, l_learn_length):
            for id_rdsom,id_traj in it.product( range(nb_rdsom), range(nb_traj)):
                hmm_name = "data_hmm/hmm_"+hmm_expr+".json"
                traj_name = "data_hmm/traj_"+hmm_expr+"_"+str(traj_size)+"_n{0:03d}".format( id_traj )+".data"
                rdsom_name = "data_rdsom/rdsom_"+str(rdsom_size)
                rdsom_name += "_n{0:03d}".format( id_rdsom )+".json"
                ## allow saving RDSOMs
                output_name = hmm_expr+"_"+str(traj_size)+"_"+str(rdsom_size)+"_"+str(ela)+"_"+str(ela_rec)+"_"+str(eps)+"_"+str(sig_i)+"_"+str(sig_r)+"_"+str(beta)+"_"+str(learn_length)
                output_name += "_rdsom{0:03d}".format(id_rdsom)+"_t{0:03d}".format(id_traj)

                output_name = "data_rdsom/New/result_"+output_name+".data"
                ## Run XP
                repeat( name_hmm = hmm_name,
                        name_traj=    traj_name,
                        name_rdsom=     rdsom_name,
                        ela = ela,
                        ela_rec = ela_rec,
                        eps = eps,
                        sig_i = sig_i,
                        sig_r = sig_r,
                        beta = beta,
                        learn_length = learn_length,
                        name_output = output_name,
                        queue_size = s_queue_size,
                        period_save = s_period,
                        nb_repeat = nb_repeat,
                        idx_start = nb_start
                )
            id_xp += 1
            pbar.update( id_xp )
        pbar.finish()

    if fg_test:
        # ## Test pour toutes les combinaisons de RDSOM x TRAJ
        nb_combination = len(l_hmm_names)*len(l_traj_size)*len(l_rdsom_size)*len(l_ela)*len(l_ela_rec)*len(l_eps)*len(l_sig_i)*len(l_sig_r)*len(l_beta)*len(l_learn_length)*len(l_hmm_names_test)*len(l_traj_size_test)
        print "__TEST","  "+str(nb_rdsom*nb_traj)+" x nb_config="+str(nb_combination)
        pbar = pb.ProgressBar(maxval=nb_combination,
                              widgets = ['  ',pb.SimpleProgress(), ' ', pb.Bar()]).start()
        id_xp = 0
        
        for hmm_expr,traj_size,rdsom_size,ela,ela_rec,eps,sig_i,sig_r,beta,learn_length, hmm_expr_test,traj_size_test in it.product( l_hmm_names, l_traj_size, l_rdsom_size, l_ela, l_ela_rec, l_eps, l_sig_i, l_sig_r, l_beta, l_learn_length, l_hmm_names_test, l_traj_size_test):
            for id_rdsom,id_traj_in,id_traj_test in it.product( range(nb_rdsom), range(nb_traj), range(nb_traj_test)):
                hmm_name = "data_hmm/hmm_"+hmm_expr+".json"
                traj_name_test = "data_hmm/traj_"+hmm_expr_test+"_"+str(traj_size_test)+"_n{0:03d}".format( id_traj_test )+".data"

                ## allow saving RDSOMs
                input_name = hmm_expr+"_"+str(traj_size)+"_"+str(rdsom_size)+"_"+str(ela)+"_"+str(ela_rec)+"_"+str(eps)+"_"+str(sig_i)+"_"+str(sig_r)+"_"+str(beta)+"_"+str(learn_length)
                input_name += "_rdsom{0:03d}".format(id_rdsom)+"_t{0:03d}".format(id_traj_in)
                input_name += ".data_000_rdsom_"+str(learn_length)             
                input_name = "data_rdsom/New/result_"+input_name
                
                output_name = input_name+"_test_"+hmm_expr_test+"_"+str(traj_size_test)+"_n{0:03d}".format( id_traj_test)
                ## Run TEST
                # print ( "__TEST "+input_name )
                # print ( "  with "+traj_name_test )
                repeat( name_hmm = hmm_name,
                        name_traj=    traj_name_test,
                        name_rdsom=     input_name,
                        ela = ela,
                        ela_rec = ela_rec,
                        eps = eps,
                        sig_i = sig_i,
                        sig_r = sig_r,
                        beta = beta,
                        learn_length = learn_length,
                        name_output = output_name,
                        queue_size = s_queue_size,
                        period_save = s_period,
                        nb_repeat = nb_repeat,
                        idx_start = nb_start,
                        testing = True,
                        nb_test = s_nb_test
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

    
