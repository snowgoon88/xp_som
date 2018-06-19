# utilise ggplot
require(ggplot2)
source("r_scripts/utils.R")

###############################################################################
## Visualiser les tests de RDSOM
##
## Exemple : lire et ploter test de p01BCDEDC sur BCDEDC
## sname <- "data_xprdsom/New/result_BCDEDC_600_50_1.0_0.005_0.25_0.1_0.05_0.05_0.05_20000_rdsom000_t000.data_000_rdsom_20000_test_"
## ename <- "_000_test_000"
## df1 <- load_err_pred( sname, "p01BCDEDC", ename, 10)
## df1 <- compute_stats(df1)
## p_root <- ggplot()
## p_err1 <- plot_err_pred(df1, "p01")
## p_root+p_err1[1]+p_err1[2]+coord_cartesian(xlim=c(10,100))+theme(legend.position = c(0.1,0.89))
##
## version PDF
## to_pdf( "try.pdf", p_root+p_err1[1]+p_err1[2]+coord_cartesian(xlim=c(10,100))+theme(legend.position=c(0.1,0.89)) ) )
###############################################################################

###############################################################################
## Load all 'err_pred' from files into one dataframe
## - startname = result_BCDEDC_600_50_1.0_0.005_0.25_0.1_0.05_0.05_0.05_20000_rdsom000_t000.data_000_rdsom_20000_test_
## - label = p01BCDEDC
## - then insert _1000_t
## - "00x" is inserted for i in 0:(nbfiles-1)
## - endname = _000_test_000
##
## with names(data)= "ite", "label", "w_in0", "w_pred0", "e_pred0", etc
load_err_pred <- function( startname, label, endname, nbfiles )
{
  # initial data
  dread <- read.table( paste(startname, label, "_1000_t",
                             "000", endname, sep=""),
                       header=TRUE)
  data <- as.data.frame( dread$ite )
  # then add a column with label
  data$label <- rep( label, length(data[,1]))
  
  ## add all err_pred
  # list of all the files name "rootname+00x"
  files <- paste( startname,
                  label, "_1000_t",
                  formatC(0:(nbfiles-1), width=3, flag="0"), 
                  endname, sep="")
  # list of columns names w_in0,w_pred0,e_pred0, w_in1,w_pred1,e_pred1,...
  colnames <- paste( c("w_in","w_pred","e_pred"), rep(0:(nbfiles-1), each=3), sep="")

  ## Function to add w_pred AND err_pred to dataframe
  add_err_pred.d <- function( name ) {
    print( paste("R:",name))
    dread <- read.table( file=name, header=TRUE )
    data <<- cbind( data, dread$input)
    data <<- cbind( data, dread$w_predwin)
    data <<- cbind( data, dread$err_pred)
  }
  lapply( files, add_err_pred.d )
  # rename columns
  names(data) <- c("ite","label",colnames)

  return(data)
}
###############################################################################
## - startname = "result_BCDEDC_600_50_1.0_0.005_0.25_0.1_0.05_0.05_0.05_20000_rdsom"
## - ename = "_t000.data_000_errors"
## - then insert "00x" for i in 0:(nbfiles-1)
##
## with names(data) = "ite", epred0, e_pre1, ...
load_err_pred_learn <- function( startname, label, endname, nbfiles )
{
    
  # initial data
  dread <- read.table( paste(startname,
                             "000", endname, sep=""),
                      header=TRUE)
  data <- as.data.frame( dread$ite )
  # then add a column with label
  data$label <- rep( label, length(data[,1]))

  ## add all err_pred
  # list of all the files name "rootname+00x"
  files <- paste( startname,
                  formatC(0:(nbfiles-1), width=3, flag="0"), 
                  endname, sep="")
  # list of columns names epred0, epred1, etc
  colnames <- paste( c("epred"), rep(0:(nbfiles-1), each=1), sep="")
  print( colnames )

  ## Function to err_pred to dataframe
  add_err_pred.d <- function( name ) {
    print( paste("R:",name))
    dread <- read.table( file=name, header=TRUE )
    data <<- cbind( data, dread$err_pred)
  }
  lapply( files, add_err_pred.d )
  # rename columns
  names(data) <- c("ite","label",colnames)

  return(data)  
}

###############################################################################
## compute mean and sd of data
## new columns e_pred_mean e_pred_sd
## WARN: only if data is "it","e_pred0",...,"e_predn"
## mean of column N° idx.start, idx.start+idx.stride, idx.start+2*idx.stride, ...
compute_stats <- function( data, idx.start=5, idx.stride=3)
{
  col_idx <- seq( from=idx.start, to=length(data), by=idx.stride)
  # mean of SEVERAL colums
  if (is.null( dim(data[,col_idx]))) {
    data$e_pred_mean = data[,idx.start]
    data$e_pred_sd = rep( 0, length(data$ite))
  }
  else {
    # mean and sd are not applied to first column (ite)
    data$e_pred_mean = apply(data[,col_idx], 1, mean)
    data$e_pred_sd = apply(data[,col_idx], 1, sd)
  }
  return(data)
}
###############################################################################

###############################################################################
## make plot ite vs mean+/-sd for e_pred
## - data: dataframe with cols like e_pred_mean and e_pred_sd
##
## => (p_err_pred_mean, p_err_pred_sd)
##
plot_err_pred <- function(data)
{
  p_err_pred_sd <- geom_ribbon(data=data,
                               mapping=aes( x=ite,
                                            ymin=(e_pred_mean-e_pred_sd),
                                            ymax=(e_pred_mean+e_pred_sd),
                                            fill=label),
                               show.legend = FALSE,
                               alpha=0.5)
  p_err_pred_mean <- geom_line(data=data,
                                mapping=aes(x=ite, y=e_pred_mean, color=label)
                                )
   return( list(p_err_pred_mean, p_err_pred_sd))
}
###############################################################################

###############################################################################
## make plot trajectories : w_pred vs w_in
## - data: dataframe with cols like w_inX and w_predX
## - idx_traj : index of trajctory we want to plot
##
## => (p_tra_in, p_traj_pred)
##
plot_traj <- function(data, idx_traj )
{
  print( paste( "plot_traj with idx=", idx_traj))
  col_in <- paste( "w_in", idx_traj, sep="")
  col_pred <- paste( "w_pred", idx_traj, sep="")
  p_traj_in <- geom_line( data=data,
                          mapping=aes_( x=quote(ite),
                                        y=as.name(col_in),
                                        color=col_in)
                          )
  p_traj_pred <- geom_line( data=data,
                            mapping=aes_( x=quote(ite),
                                          y=as.name(col_pred),
                                          color=col_pred)
  )
  return( list(p_traj_in,p_traj_pred))
}
###############################################################################

###############################################################################
## Effective multiplot of all in/pred trajectories using 2 columns
## - data : dataframe with w_inX, w_predX columns
## - idx_all : sequence of trajectory indexes
## - ... : gplot extra commands for each subplot
##
## Example : plot_all_traj(df3[1:200,], 0:9, coord_cartesian(xlim=c(10,100)), 
##             theme(axis.title.x = element_blank(), axis.title.y = element_blank()) )
##
plot_all_traj <- function( data, idx_all, ... )
{
    ## base for plotting
    p_root <- ggplot()
    gplot_extra <- list(...)
    ## list of all plots
    all_plots <- lapply( idx_all,
                        function(x) {
                            ptraj <- plot_traj( data, x )
                            pl <- p_root+ptraj[1]+ptraj[2]
                            for( gp in gplot_extra ) {
                              pl <- pl + gp
                            }
                            return(pl)
                        }
                        )
    multiplot( plotlist=all_plots, cols=2 )
}
###############################################################################

## AF__p__BCDEDC
## p_root+perra05[1]+perra95[1]+perr1[1]+perra05[2]+perra95[2]+perr1[2]+coord_cartesian(xlim=c(10,100))+theme(legend.position = c(0.35,0.89))
## BCDEDC__p__s
## p_root+perrp10[1]+perrp05[1]+perr1[1]+perrp10[2]+perrp05[2]+perr1[2]+coord_cartesian(xlim=c(10,100))+theme(legend.position = c(0.1,0.89))

## plot traj
## plot_all_traj(df3[1:200,], 0:9, coord_cartesian(xlim=c(10,100)), theme(axis.title.x = element_blank(), axis.title.y = element_blank()) )
## plot_all_traj(df5[1:200,], 0:9, coord_cartesian(xlim=c(10,100)), theme(axis.title.x = element_blank(), axis.title.y = element_blank()) )
## proot+pt5.0[1]+pt5.0[2]+coord_cartesian(xlim=c(10,100))
## proot+pt5.0[1]+pt5.0[2]+coord_cartesian(xlim=c(45,65))

## pl <- proot+pt5.0[1]+pt5.0[2]+coord_cartesian(xlim=c(45,65))
## plot_adapt <- function( plot, title=NA, x=NA, y=NA, textsize=1, leg_rel=0.75, leg_pos=NULL)
