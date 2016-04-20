# utilise ggplot
require(ggplot2)

# Lit les résultats
res <- read.table(file= "result", header=TRUE)
# vecteur d'indices de 1 à nbState
nbState <- 11
rg <- seq(1,nbState)
# ajoute la position des max
res$ta_idx <- apply(res[,rg], 1, which.max)
res$le_idx <- apply(res[,nbState+rg], 1, which.max)
res$in_idx <- apply(res[,2*nbState+rg], 1, which.max)
# teste le nb de bonne réponses
sum(res$ta_idx == res$le_idx)
sum(res$ta_idx == res$in_idx)
# Calcule la somme des carrés de l'erreur entre ta et le
mse1 <- function(row) {sum((row[rg] - row[rg+nbState])*(row[rg] - row[rg+nbState]))}
# Calcule la somme des carrés de l'erreur entre ta et in
mse2 <- function(row) {sum((row[rg] - row[rg+2*nbState])*(row[rg] - row[rg+2*nbState]))}
# ajoute dans la dataframe
# WARN à faire avant que des charactères soient utilisés dans dataframe
res$le_mse <- apply( res, 1, mse1)
res$in_mse <- apply( res, 1, mse2)

# un label suivant "qui" a bien appris
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
# labellise les données
res$label <- mapply(label_learn,res$ta_idx, res$le_idx, res$in_idx)

##################################################################
# plot Erreur en LEARN vs Erreur en INIT
# Ce qui est au dessus de la diagonale est mieux appris par LEARN
# TODO Couleur en fonction de si le résultat est bon.
# root layer
p_root <- ggplot(res)
# Chacune des lignes, avec la couleur qui dépend de son nom
p_scatter <- geom_point( aes( x=le_mse, y=in_mse, color=label, shape=label), size=5)
# ligne diagonale
p_lin <- geom_abline( slope=1, intercept=0)
# Choix manuel des couleurs utilisées en vrai (ordre est important)
# p_col <- scale_color_manual( values=c('red','green'))
# Titre, axes et nom de la legende de couleur
p_labs <- labs(colour=NULL, shape=NULL, title="LEARN vs INIT", x="le", y="in")
# Put upper-right corner of legend box in upper-right corner of graph
# p_leg <- theme(legend.justification=c(1,1), legend.position=c(1,1))
# AU FINAL
p_root + p_scatter + p_lin + p_labs

##################################################################
# plot nb de bonnes réponses
# nb cumulé de bonnes réponses
res$index <- seq(1,dim(res)[1])
res$nb_in <- cumsum( res$label == "init" | res$label == "both")
res$nb_le <- cumsum( res$label == "learn" | res$label == "both")
#
p_nbin <- geom_line( aes( x=index, y=nb_in, color="init"))
p_nble <- geom_line( aes( x=index, y=nb_le, color="learn"))
# Titre, axes et nom de la legende de couleur
p_labs2 <- labs(colour=NULL, title="Cumulative nb of good answer", x="index", y="CumNb")
p_root + p_nbin + p_nble + p_labs2

##################################################################
# plot nb bonne réponse en fonction de state
p_root <- ggplot(res)
# compter - histogramme des états ; répartition des cas par état
p_bin_tot <- geom_bar( aes(x=ta_idx, y = ..count..), colour="black", fill="white")
p_bin_in <- geom_bar( aes(x=ta_idx, y = ..count.., fill=label), position=position_dodge())
# plot
p_root + p_bin_tot + p_bin_in

##################################################################
# Analyser les résultats
# fichiers
fich <- list.files( path="data_xp", pattern="result_*")
items <- strsplit( fich[1], '_')

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
## Trouver les min/max de chaque colonne
###############################################################################
## Faire les moyenne
attach(df.sum)
df.mean <- aggregate(df.sum[,c("rate_le","rate_in","mse_le","mse_in")], by=list(ltraj,lesn,leak,regul), FUN=mean)
detach( df.sum )
## Remplacer "Groupe.1" par son 'vrai nom
names(df.mean)[1:4] <- c("ltraj","lesn","leak","regul")
## trouver le max de rate_le
attach( df.mean )
which.max( rate_le )
## etc

###############################################################################
## Une façon générique de tracer les courbes autour d'un point
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