# utilise ggplot
require(ggplot2)

###############################################################################
## df <- make_df_hmm( "data_hmm" )
## attach(df)
## idx_best <- order( mse_err )
## plot_many_traj( df, idx_best[1:10], "Pic/")
###############################################################################

###############################################################################
## Préparer une data.frame avec un résumé des données
## :format: filename,basename,ltraj,lesn,leak,regul,noesn,notraj,norun,
##          
###############################################################################
make_df_hmm <- function( path ) {
    ## all 'result_' files from path
    fich <- list.files( path=path, pattern="result_")
    ## empty df
    df <- data.frame( filename=character(), basename=character(),
                     hmm=character(),
                     ltraj=numeric(), lesn=numeric(),
                     leak=numeric(), regul=numeric(),
                     fw=character(), lnoise=numeric(),
                     ltest=numeric(), type=character(),
                     noesn=character(), notraj=character(),
                     nonoise=character(),
                     norun=character(),
                     
                     prec_err=numeric(), mse_err=numeric())
    # fonction a appliquer à chaque nom
    mk_dfrow <- function( filename ) {
        info <- extract( filename )
        ## read data
        name <- paste( path, filename, sep='/')
        data <- read.table( file=name, header=TRUE )
        data <- add_low_level( data )
        stat <- compute_stat( data)
        dfrow <- merge( info, stat)
        df <<- rbind( df, dfrow )
    }
    ## apply to all names
    lapply( fich, mk_dfrow )
    return( df )
}

###############################################################################
## Plot MANY trajectory
###############################################################################
plot_many_traj <- function( df, l_indices, rep="" ) 
{
  for(row in l_indices) {
    str_tit <- paste( df[row,1], " MSE=", df[row,17], sep="")
    str_file <- paste( "data_hmm", df[row,1], sep="/")
    png( filename = paste( rep, df[row,1], ".png", sep=""),
          width = 800, height=600)
    print( plot_one_traj( str_file, str_tit) )
    dev.off()
  }
  ##apply( df[l_indices,], 1, make_plot)
}
###############################################################################
## Plot ONE trajectory
###############################################################################
plot_one_traj <- function( filename, tit=NULL )
{
  p_root <- ggplot()
  # read file and compute states
  data <- read.table( file=filename, header=TRUE )
  # length
  data$index <- seq(length(data$ta_0))
  p_target <- geom_line( data=data,
                         aes( x=index, y=ta_0, color='target'))
  p_output <- geom_line( data=data,
                         aes( x=index, y=le_0, color='output'))
  
  pl <- p_root+p_target+p_output
  
  # Ajoute titre
  if( !is.null(tit) ) {
    pl <- pl +  ggtitle( tit )
  }
  ##l_plots <<-append( l_plots, pl)
  return(pl)
}




###############################################################################
## Extract parameters and basename from ONE filename
## result_hhmmexp_trajsize_esnsize_leak_regul(_for)_eXXX_tXXX_noiselength_nXXX
###      _lXXX.data_XXX_learn/test
## :return: data.frame
###############################################################################
extract <- function( filename ) {
    ## Hyp : only one filename
    items <- strsplit( filename, split='_')[[1]]
    hmm <-items[2]
    ltraj <- as.numeric( items[3] )
    lesn <- as.numeric( items[4] )
    leak <- as.numeric( items[5] )
    regul <- as.numeric( items[6] )
    offset <- 0
    fw <- 'N'
    if( items[7] == 'for' ) {
        fw <- 'Y'
        offset <- 1
    }
    noesn <- items[7+offset]
    notraj <- items[8+offset]
    lnoise <- as.numeric( items[9+offset] )
    nonoise <- items[10+offset]
    # Warning : '.' is also a regexp, so set fixed=TRUE
    subitems <- strsplit( items[11+offset], split='.', fixed=TRUE)[[1]]
    ltest <- as.numeric( substr(subitems[1],2,nchar(subitems[1]) ) )
    norun <- items[12+offset]
    type <- items[13+offset]
    ## 'basename' is only a function of ltraj,lesn,leak,regul
    basename <- paste( hmm,ltraj,lesn,leak,regul,fw,lnoise,ltest, sep='_')
    return( data.frame(filename,basename, hmm, ltraj, lesn, leak, regul, fw, lnoise, ltest, type, noesn, notraj, nonoise, norun))
}
###############################################################################
## Compute MSE and Precision
## :param: dataframe 
###############################################################################
## Compute mse for a row ($ta_[rg], $le_[rg])
nbState <- 1
rg <- seq(1,nbState) 
mse <- function(row) {
    sum((row[rg] - row[rg+nbState])*(row[rg] - row[rg+nbState]))
}
## True if output within 0.1 of target
prec <- function(row) {
  abs(row[1] - row[2]) < 0.1
}
## Add to low level dataframe
add_low_level = function( data ) {
    
  # Compute MSE error  
  # WARN to do BEFORE characters used in dataframe
  data$mse <- apply( data, 1, mse)
  # precision as TRYE/FALSE
  data$prec <- apply( data, 1, prec)
  
  return (data)
}
## Compute stat from lowlevel data
compute_stat <- function( data ) {
  # Nb of samples
  nb_sample = dim(data)[1]
  # Precision
  prec_err <- sum( data$prec ) / nb_sample
  # MSE
  mse_err <- sum( data$mse ) / nb_sample
  
  return( data.frame( prec_err, mse_err ))
}

df <- make_df_hmm( "data_hmm" )
plot_many_traj( df, c(87,940), "Pic/")




