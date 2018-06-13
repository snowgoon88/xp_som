# utilise ggplot
require(ggplot2)

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
## with names(data)= "ite", "e_pred0", "e_pred1", etc
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
  # list of columns names
  colnames <- paste( "e_pred", 0:(nbfiles-1), sep="")

  ## Function to add err_pred to dataframe
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

###############################################################################
## compute mean and sd of data
## new columns e_pred_mean e_pred_sd
## WARN: only if data is "it","e_pred0",...,"e_predn"
compute_stats <- function( data )
{
  # mean and sd are not applied to first column (ite)
  data$e_pred_mean = apply(data[,-2:-1], 1, mean)
  data$e_pred_sd = apply(data[,-2:-1], 1, sd)
  return(data)
}
###############################################################################

###############################################################################
## plot ite vs mean+/-sd for e_pred
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
## plot in a pdf
to_pdf <- function( filename, gplot )
{
  # Must use print( cmd...) otherwise, nothing is plotted in script mode
  # Defaut size is 7x7 inches
  pdf( file=filename )
  print( gplot )
  dev.off()
}
###############################################################################

## AF__p__BCDEDC
## p_root+perra05[1]+perra95[1]+perr1[1]+perra05[2]+perra95[2]+perr1[2]+coord_cartesian(xlim=c(10,100))+theme(legend.position = c(0.35,0.89))
## BCDEDC__p__s
## p_root+perrp10[1]+perrp05[1]+perr1[1]+perrp10[2]+perrp05[2]+perr1[2]+coord_cartesian(xlim=c(10,100))+theme(legend.position = c(0.1,0.89))


