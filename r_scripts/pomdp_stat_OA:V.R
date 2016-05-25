###############################################################################
## Ajoute info bas niveau dans data
###############################################################################
## Var/FUN globales
# vecteur d'indices de 1 à nbState
nbState <- 1
rg <- seq(1,nbState) 
## un label suivant "qui" a bien appris
#  - both : init et learn
label_learn <- function(target, learn ) {
  if(target == learn) {
    return ("learn")
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
  
  # Calcules les erreurs MSE et ajoute dans la dataframe
  # WARN à faire avant que des charactères soient utilisés dans dataframe
  data$le_mse <- apply( data, 1, mse1)
  ##data$in_mse <- apply( data, 1, mse2)
  
  # labellise les données en "both/init/learn/none"
  # data$label <- mapply(label_learn, data$ta_idx, data$le_idx )
  
  return (data)
}

###############################################################################
## Calcule le taux de bonne réponse et le taux d'erreur
## :format: rate_le, rate_in, mse_le, mse_in
###############################################################################
compute_stat <- function( data ) {
  # Nb of samples
  nb_sample = dim(data)[1]
  # MSE
  rate_mse_le = sum( data$le_mse ) / nb_sample
  ##rate_mse_in = sum( data$in_mse ) / nb_sample
  
  return( data.frame(mse_le=rate_mse_le))
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
  notraj <- items[7]
  # Warning : '.' is also a regexp, so set fixed=TRUE
  subitems <- strsplit( items[8], split='.', fixed=TRUE)[[1]]
  ltest <- as.numeric( substr(subitems[1],2,nchar(subitems[1]) ) )
  norun <- items[9]
  type <- items[10]
  ## 'basename' is only a function of ltraj,lesn,leak,regul
  basename <- paste( ltraj,lesn,leak,regul,ltest, sep='_')
  return( data.frame(filename,basename,ltraj, lesn, leak, regul, ltest, type, noesn, notraj, norun))
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
                    ltest=numeric(), type=character(),
                    noesn=character(), notraj=character(),
                    norun=character(),
                    mse_le=numeric())
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
## Plot around a 5dim-PT
## pt=c(ltraj,lesn,leak,regul,ltest) with 'NA' where variation is expected
##
## ex: plot_pt_all( df.sum, df.mean, c(500,50,NA,0.1,100))
##     to plot with varying 'leak'
###############################################################################
plot_pt_all <- function ( dsum, dmean, point, ylim.sum=c(0,1), ylim.mean=c(0,10) ) {
  ## construire la condition
  ## pt = c(num|NA,...)
  bd_cond <- function( pt ) {
    base_str <- c( "ltraj", "lesn", "leak", "regul", "ltest")
    idx_str  <- c("1","2","3","4","5")
    abs_str <- base_str[is.na(pt)]
    dsum_str <- paste( "dsum$", base_str[!is.na(pt)],
                       "==point[", idx_str[!is.na(pt)], "]", sep="", collapse=" & ")
    dmean_str <- paste( "dmean$", base_str[!is.na(pt)],
                        "==point[", idx_str[!is.na(pt)], "]", sep="", collapse=" & ")
    title_str <- paste( base_str[!is.na(pt)],
                        "=", pt[!is.na(pt)], sep="", collapse=", ")
    #   cond_expr <- eval(parse(text=cond_str))
    #   abs_expr <- eval(parse(text=abs_str))
    return( list(abs_str,dsum_str,dmean_str,title_str))#,cond_expr,abs_expr))               
  }
  all_str <- bd_cond( point )
  ## PLOT
  p_root <- ggplot()
  subdsum <- subset(dsum, eval(parse(text=all_str[2])))
  p_ratele <- geom_point( data=subdsum,
                          aes( x=eval(parse(text=all_str[1])), y=rate_le))
  p_msele <- geom_point( data=subdsum, 
                         aes( x=eval(parse(text=all_str[1])), y=mse_le))
  
  subdmean <- subset(dmean, eval(parse(text=all_str[3])))
  p_ratele_ln <- geom_line( data=subdmean,
                            aes( x=eval(parse(text=all_str[1])), y=rate_le))
  p_msele_ln <- geom_line( data=subdmean,
                           aes( x=eval(parse(text=all_str[1])), y=mse_le))
  
  p_coord1 <- coord_cartesian(ylim = ylim.sum)
  p_labs1 <- labs(colour=NULL, title=paste("Success Rate",all_str[4]),
                  x=all_str[1], y="rate")
  p1 <- p_root + p_ratele + p_ratele_ln + p_labs1 + p_coord1
  p_coord2 <- coord_cartesian(ylim = ylim.mean) 
  p_labs2 <- labs(colour=NULL, title=paste("MSE Error",all_str[4]),
                  x=all_str[1], y="mse")
  p2 <- p_root + p_msele + p_msele_ln + p_labs2 + p_coord2
  multiplot( p1, p2, cols=1 )
}


###############################################################################
## Subset de dsum qui sont compatible avec le point donné
## point = list(ltraj,leasn,leak,regul,ltest,type)
##       où chaque élément peut être NA
###############################################################################
get_filenames <- function(dsum,  point ) {
  res <- mk_query( point )
  subd <- subset( dsum, eval(parse(text=res[2])))
  return( subd )
}
## Liste tous les noms de fichiers qui sont compatible avec la requete
mk_query <- function( pt ) {
  base_str <- c( "ltraj", "lesn", "leak", "regul", "ltest","type")
  idx_str  <- c("1","2","3","4","5","6")
  abs_str <- base_str[is.na(pt)]
  dsum_str <- paste( "dsum$", base_str[!is.na(pt)],
                     "==point[", idx_str[!is.na(pt)], "]", sep="", collapse=" & ")
  title_str <- paste( base_str[!is.na(pt)],
                      "=", pt[!is.na(pt)], sep="", collapse=", ")
  #   cond_expr <- eval(parse(text=cond_str))
  #   abs_expr <- eval(parse(text=abs_str))
  return( list(abs_str,dsum_str,title_str))#,cond_expr,abs_expr))               
}

###############################################################################
## Plot toutes les trajectoires pour un point donné et un esn
## - subd : une sous data.frame (voir get_filenames )
## - esn : le numéro de l'esn à visualiser
## - str.title : le titre du plot, souvent par mk_query
###############################################################################
plot_traj_esn <- function( subd, esn=0, str.title=NULL ) {
  print(paste("TIT:",str.title))
  # Le nom du repertoire
  str.dir <- "data_v"
  # le nom de l'extension
  str.esn <- paste( "e", formatC(esn, width=3, flag="0"), sep="")
  # Les fichiers
  filenames <- paste( str.dir, "/",
                      subd$filename[subd$noesn==str.esn], sep="")
  score <- paste( ":rate=", subd$rate_le[subd$noesn==str.esn],
                  " mse=", formatC(subd$mse_le[subd$noesn==str.esn], width=4),
                  sep="")
  #
  # Crée une liste de plot pour utiliser multiplot
  l_plots <- list()
  add_plot_traj <- function (filename, tit=NULL) {
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
      p1 <<- p1 +  ggtitle( tit )
    }
    ##l_plots <<-append( l_plots, pl)
    return(pl)
  }
  ## apply to all filenames
  ##l_plots[[1]] <- add_plot_traj( filenames[1], tit=str.title )
  for( i in 1:length(filenames)) {
    pn <- add_plot_traj( filenames[i] )
    if( i==1 ) {
      pn <- pn + ggtitle( paste( str.title, "esn=", esn, score[[i]] ))
      
    }
    else {
      pn <- pn + ggtitle( score[[i]] )
    }
    l_plots[[i]] <- pn
  }
  multiplot( plotlist=l_plots, cols=1)
  ##return( l_plots )
}

###############################################################################
## Fonction intégrée pour faciliter l'utilisation de
## plot_traj_esn
###############################################################################
look_traj <- function( dsum, point, esn=0 ) {
  tit <- mk_query( point )[[3]]
  sub <- get_filenames( dsum, point )
  plot_traj_esn( sub, esn=esn, str.title=tit )
  return( list(sub, tit, point) )
}

###############################################################################
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

