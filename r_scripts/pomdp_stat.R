###############################################################################
## Ajoute info bas niveau dans data
###############################################################################
## Var/FUN globales
# vecteur d'indices de 1 à nbState
nbState <- 11
rg <- seq(1,nbState) 
## un label suivant "qui" a bien appris
#  - both : init et learn
label_learn <- function(target, learn, init) {
  if (target == learn && target == init) {
    return ("both")
  }
  else if(target == learn) {
    return ("learn")
  }
  else if(target == init) {
    return ("init")
  }
  else {
    return ("none")
  }
}
## Calcule la somme des carrés de l'erreur entre ta et le
mse1 <- function(row) {sum((row[rg] - row[rg+nbState])*(row[rg] - row[rg+nbState]))}
## Calcule la somme des carrés de l'erreur entre ta et in
mse2 <- function(row) {sum((row[rg] - row[rg+2*nbState])*(row[rg] - row[rg+2*nbState]))}
## modification du data_frame
add_low_level = function( data ) {
#   # vecteur d'indices de 1 à nbState
#   nbState <- 11
#   rg <- seq(1,nbState)
  # ajoute la position des max
  data$ta_idx <- apply(data[,rg], 1, which.max)
  data$le_idx <- apply(data[,nbState+rg], 1, which.max)
  data$in_idx <- apply(data[,2*nbState+rg], 1, which.max)
  
  # Calcules les erreurs MSE et ajoute dans la dataframe
  # WARN à faire avant que des charactères soient utilisés dans dataframe
  data$le_mse <- apply( data, 1, mse1)
  data$in_mse <- apply( data, 1, mse2)
  
  # labellise les données en "both/init/learn/none"
  data$label <- mapply(label_learn, data$ta_idx, data$le_idx, data$in_idx)
  
  return (data)
}

###############################################################################
## Calcule le taux de bonne réponse et le taux d'erreur
## :format: rate_le, rate_in, mse_le, mse_in
###############################################################################
compute_stat <- function( data ) {
  # Nb of samples
  nb_sample = dim(data)[1]
  # Rate of error
  rate_err_le = sum( data$label == "learn" | data$label == "both") / nb_sample
  rate_err_in = sum( data$label == "init" | data$label == "both") / nb_sample
  
  # MSE
  rate_mse_le = sum( data$le_mse ) / nb_sample
  rate_mse_in = sum( data$in_mse ) / nb_sample
  
  return( data.frame(rate_le=rate_err_le, rate_in=rate_err_in, 
            mse_le=rate_mse_le, mse_in=rate_mse_in))
}

###############################################################################
## Fait la moyenne des stats sur nb_run XP
## => Pas très utile pour l'instant : déterministe.
###############################################################################
pomdp_stat_xp <- function( basename, nb_run ) {
  ## Generate names
  seq <- 0:(nb_run-1)
  names <- paste( paste(basename,"_",sep=""), formatC(seq, width=3, flag="0"), sep="")
  ## Empty DataFrame
  stat.d <- data.frame( rate_le=double(), rate_in=double(), 
                        mse_le=double(), mse_in=double(),
                        xp=character())
  ## Function to add to dataframe
  fill_stat.d <- function( name ) {
    #print( paste( "__stat for",name))
    data <- read.table( file=name, header=TRUE )
    data <- add_low_level( data )
    stat <- compute_stat( data)
    #print( paste( "  ", stat))
    stat$xp = basename
    stat.d <<- rbind( stat.d, stat)
  }
  lapply( names, fill_stat.d )
  return( stat.d )
}

###############################################################################
## Extract parameters and basename from ONE filename
## :return: data.frame
###############################################################################
extract <- function( filename ) {
  ## Hyp : only one filename
  items <- strsplit( filename, split='_')[[1]]
  ltraj <- as.numeric( items[2] )
  lesn <- as.numeric( items[3] )
  leak <- as.numeric( items[4] )
  regul <- as.numeric( items[5] )
  noesn <- items[6]
  # Warning : '.' is also a regexp, so set fixed=TRUE
  subitems <- strsplit( items[7], split='.', fixed=TRUE)[[1]]
  notraj <- subitems[1]
  norun <- items[8]
  ## 'basename' is only a function of ltraj,lesn,leak,regul
  basename <- paste( ltraj,lesn,leak,regul, sep='_')
  return( data.frame(filename,basename,ltraj, lesn, leak, regul, noesn, notraj, norun))
}

###############################################################################
## Préparer une data.frame avec un résumé des données
## :format: filename,basename,ltraj,lesn,leak,regul,noesn,notraj,norun,
##          
###############################################################################
make_df_pomdp <- function( path ) {
  ## all 'result_' files from path
  fich <- list.files( path=path, pattern="result_")
  ## empty df
  df <- data.frame( filename=character(), basename=character(),
                    ltraj=numeric(), lesn=numeric(),
                    leak=numeric(), regul=numeric(),
                    noesn=character(), notraj=character(),
                    norun=character(),
                    rate_le=numeric(), rate_in=numeric(),
                    mse_le=numeric(), mse_in=numeric())
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
## Plot around a 4dim-PT
###############################################################################
plot_regul_all<- function ( dsum, dmean, point ) {
  p_root <- ggplot()
  
  subdsum <- dsum[dsum$ltraj == point[1] &
                    dsum$lesn==point[2] &
                    dsum$leak==point[3],]
  p_ratele <- geom_point( data=subdsum, aes( x=regul, y=rate_le, color='le'))
  p_ratein <- geom_point( data=subdsum, aes( x=regul, y=rate_in, color='in'))
  p_msele <- geom_point( data=subdsum, aes( x=regul, y=mse_le, color='le'))
  p_msein <- geom_point( data=subdsum, aes( x=regul, y=mse_in, color='in'))
  #p_ratele_sm <- geom_smooth( data=subdsum,aes( x=regul, y=rate_le))

  subdmean<- dmean[dmean$ltraj == point[1] &
                     dmean$lesn==point[2] &
                     dmean$leak==point[3],]
  p_ratele_ln <- geom_line( data=subdmean, aes( x=regul, y=rate_le, color='le'))
  p_ratein_ln <- geom_line( data=subdmean, aes( x=regul, y=rate_in, color='in'))
  p_msele_ln <- geom_line( data=subdmean, aes( x=regul, y=mse_le, color='le'))
  p_msein_ln <- geom_line( data=subdmean, aes( x=regul, y=mse_in, color='in'))

  p_coord1 <- coord_cartesian(ylim = c(0, 1)) 
  p_labs1 <- labs(colour=NULL, title="Success Rate", x="regul", y="rate")
  p1 <- p_root + p_ratele + p_ratele_ln + p_ratein + p_ratein_ln + p_labs1 + p_coord1 + scale_x_log10()
  p_coord2 <- coord_cartesian(ylim = c(0, 10)) 
  p_labs2 <- labs(colour=NULL, title="MSE Error", x="regul", y="mse")
  p2 <- p_root + p_msele + p_msele_ln + p_msein + p_msein_ln + p_labs2 + p_coord2 + scale_x_log10()
  multiplot( p1, p2, cols=1 )
}
plot_leak_all <- function ( dsum, dmean, point ) {
  p_root <- ggplot()
  
  subdsum <- dsum[dsum$ltraj == point[1] &
                    dsum$lesn==point[2] &
                    dsum$regul==point[3],]
  p_ratele <- geom_point( data=subdsum, aes( x=leak, y=rate_le, color='le'))
  p_ratein <- geom_point( data=subdsum, aes( x=leak, y=rate_in, color='in'))
  p_msele <- geom_point( data=subdsum, aes( x=leak, y=mse_le, color='le'))
  p_msein <- geom_point( data=subdsum, aes( x=leak, y=mse_in, color='in'))
  #p_ratele_sm <- geom_smooth( data=subdsum,aes( x=regul, y=rate_le))
  
  subdmean<- dmean[dmean$ltraj == point[1] &
                     dmean$lesn==point[2] &
                     dmean$regul==point[3],]
  p_ratele_ln <- geom_line( data=subdmean, aes( x=leak, y=rate_le, color='le'))
  p_ratein_ln <- geom_line( data=subdmean, aes( x=leak, y=rate_in, color='in'))
  p_msele_ln <- geom_line( data=subdmean, aes( x=leak, y=mse_le, color='le'))
  p_msein_ln <- geom_line( data=subdmean, aes( x=leak, y=mse_in, color='in'))
  
  p_coord1 <- coord_cartesian(ylim = c(0, 1)) 
  p_labs1 <- labs(colour=NULL, title="Success Rate", x="leak", y="rate")
  p1 <- p_root + p_ratele + p_ratele_ln + p_ratein + p_ratein_ln + p_labs1 + p_coord1
  p_coord2 <- coord_cartesian(ylim = c(0, 10)) 
  p_labs2 <- labs(colour=NULL, title="MSE Error", x="leak", y="mse")
  p2 <- p_root + p_msele + p_msele_ln + p_msein + p_msein_ln + p_labs2 + p_coord2
  multiplot( p1, p2, cols=1 )
}
plot_esn_all <- function ( dsum, dmean, point ) {
  p_root <- ggplot()
  
  subdsum <- dsum[dsum$ltraj == point[1] &
                    dsum$leak==point[2] &
                    dsum$regul==point[3],]
  p_ratele <- geom_point( data=subdsum, aes( x=lesn, y=rate_le, color='le'))
  p_ratein <- geom_point( data=subdsum, aes( x=lesn, y=rate_in, color='in'))
  p_msele <- geom_point( data=subdsum, aes( x=lesn, y=mse_le, color='le'))
  p_msein <- geom_point( data=subdsum, aes( x=lesn, y=mse_in, color='in'))
  #p_ratele_sm <- geom_smooth( data=subdsum,aes( x=regul, y=rate_le))
  
  subdmean<- dmean[dmean$ltraj == point[1] &
                     dmean$leak==point[2] &
                     dmean$regul==point[3],]
  p_ratele_ln <- geom_line( data=subdmean, aes( x=lesn, y=rate_le, color='le'))
  p_ratein_ln <- geom_line( data=subdmean, aes( x=lesn, y=rate_in, color='in'))
  p_msele_ln <- geom_line( data=subdmean, aes( x=lesn, y=mse_le, color='le'))
  p_msein_ln <- geom_line( data=subdmean, aes( x=lesn, y=mse_in, color='in'))
  
  p_coord1 <- coord_cartesian(ylim = c(0, 1)) 
  p_labs1 <- labs(colour=NULL, title="Success Rate", x="lesn", y="rate")
  p1 <- p_root + p_ratele + p_ratele_ln + p_ratein + p_ratein_ln + p_labs1 + p_coord1
  p_coord2 <- coord_cartesian(ylim = c(0, 10)) 
  p_labs2 <- labs(colour=NULL, title="MSE Error", x="lesn", y="mse")
  p2 <- p_root + p_msele + p_msele_ln + p_msein + p_msein_ln + p_labs2 + p_coord2
  multiplot( p1, p2, cols=1 )
}
plot_traj_all <- function ( dsum, dmean, point ) {
  p_root <- ggplot()
  
  subdsum <- dsum[dsum$lesn == point[1] &
                    dsum$leak==point[2] &
                    dsum$regul==point[3],]
  p_ratele <- geom_point( data=subdsum, aes( x=ltraj, y=rate_le, color='le'))
  p_ratein <- geom_point( data=subdsum, aes( x=ltraj, y=rate_in, color='in'))
  p_msele <- geom_point( data=subdsum, aes( x=ltraj, y=mse_le, color='le'))
  p_msein <- geom_point( data=subdsum, aes( x=ltraj, y=mse_in, color='in'))
  #p_ratele_sm <- geom_smooth( data=subdsum,aes( x=regul, y=rate_le))
  
  subdmean<- dmean[dmean$lesn == point[1] &
                     dmean$leak==point[2] &
                     dmean$regul==point[3],]
  p_ratele_ln <- geom_line( data=subdmean, aes( x=ltraj, y=rate_le, color='le'))
  p_ratein_ln <- geom_line( data=subdmean, aes( x=ltraj, y=rate_in, color='in'))
  p_msele_ln <- geom_line( data=subdmean, aes( x=ltraj, y=mse_le, color='le'))
  p_msein_ln <- geom_line( data=subdmean, aes( x=ltraj, y=mse_in, color='in'))
  
  p_coord1 <- coord_cartesian(ylim = c(0, 1)) 
  p_labs1 <- labs(colour=NULL, title="Success Rate", x="ltraj", y="rate")
  p1 <- p_root + p_ratele + p_ratele_ln + p_ratein + p_ratein_ln + p_labs1 + p_coord1
  p_coord2 <- coord_cartesian(ylim = c(0, 10)) 
  p_labs2 <- labs(colour=NULL, title="MSE Error", x="lraj", y="mse")
  p2 <- p_root + p_msele + p_msele_ln + p_msein + p_msein_ln + p_labs2 + p_coord2
  multiplot( p1, p2, cols=1 )
}
plot_msele_regul_all<- function ( dsum, dmean, point ) {
  p_root <- ggplot()
  
  subdsum <- dsum[dsum$ltraj == point[1] &
                    dsum$lesn==point[2] &
                    dsum$leak==point[3],]
  p_msele <- geom_point( data=subdsum, aes( x=regul, y=mse_le, color='le'))
  p_msein <- geom_point( data=subdsum, aes( x=regul, y=mse_in, color='in'))
  #p_ratele_sm <- geom_smooth( data=subdsum,aes( x=regul, y=mse_le))
  
  subdmean<- dmean[dmean$ltraj == point[1] &
                     dmean$lesn==point[2] &
                     dmean$leak==point[3],]
  p_msele_ln <- geom_line( data=subdmean, aes( x=regul, y=mse_le, color='le'))
  p_msein_ln <- geom_line( data=subdmean, aes( x=regul, y=mse_in, color='in'))
  
  p_coord <- coord_cartesian(ylim = c(0, 10)) 
  p_labs <- labs(colour=NULL, title="MSE Error", x="regul", y="mse")
  p_root + p_ratele + p_ratele_ln + p_ratein + p_ratein_ln + p_labs + p_coord + scale_x_log10()
}

# Multiple plot function
#
# ggplot objects can be passed in ..., or to plotlist (as a list of ggplot objects)
# - cols:   Number of columns in layout
# - layout: A matrix specifying the layout. If present, 'cols' is ignored.
#
# If the layout is something like matrix(c(1,2,3,3), nrow=2, byrow=TRUE),
# then plot 1 will go in the upper left, 2 will go in the upper right, and
# 3 will go all the way across the bottom.
#
multiplot <- function(..., plotlist=NULL, file, cols=1, layout=NULL) {
  library(grid)
  
  # Make a list from the ... arguments and plotlist
  plots <- c(list(...), plotlist)
  
  numPlots = length(plots)
  
  # If layout is NULL, then use 'cols' to determine layout
  if (is.null(layout)) {
    # Make the panel
    # ncol: Number of columns of plots
    # nrow: Number of rows needed, calculated from # of cols
    layout <- matrix(seq(1, cols * ceiling(numPlots/cols)),
                     ncol = cols, nrow = ceiling(numPlots/cols))
  }
  
  if (numPlots==1) {
    print(plots[[1]])
    
  } else {
    # Set up the page
    grid.newpage()
    pushViewport(viewport(layout = grid.layout(nrow(layout), ncol(layout))))
    
    # Make each plot, in the correct location
    for (i in 1:numPlots) {
      # Get the i,j matrix positions of the regions that contain this subplot
      matchidx <- as.data.frame(which(layout == i, arr.ind = TRUE))
      
      print(plots[[i]], vp = viewport(layout.pos.row = matchidx$row,
                                      layout.pos.col = matchidx$col))
    }
  }
}

