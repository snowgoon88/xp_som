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

