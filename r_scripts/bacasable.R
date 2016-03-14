# installation ggplot2
install.packages("ggplot2")
install.packages("Hmisc")
# utilise ggplot
require(ggplot2)

# Lit les données d'apprentissage
mg <- read.table( "data/mg_essai.data", na.strings="None", header=TRUE)
str(mg) # pour avoir info
# affiche avec ggplot rapide
qplot( mg$X , x=seq(1,length(mg$X)), geom="line")

# Lit les données prédites
res <-read.table( "data/result.data", na.strings="None", header=TRUE)
# Prépare data.frame
df <- data.frame( index=seq(1,length(mg$X)-1), target = mg$X[-1], predict = res$X)
# PLot
# root layer
p_root <- ggplot(df)
# Chacune des lignes, avec la couleur qui dépend de son nom
p_tar <- geom_line( aes( x=index, y=target, color='target'))
p_res <- geom_line( aes( x=index, y=predict, color='predict'))
# Choix manuel des couleurs utilisées en vrai (ordre est important)
p_col <- scale_color_manual( values=c('red','green'))
# Titre, axes et nom de la legende de couleur
p_labs <- labs(colour=NULL, title="ESN prediction of Mackey-Glass", x="index", y="value")
# Put upper-right corner of legend box in upper-right corner of graph
p_leg <- theme(legend.justification=c(1,1), legend.position=c(1,1))
# AU FINAL
p_root + p_res + p_tar + p_col + p_labs + p_leg

###############################################################
## Essayer de comparer RidgeRegression
###############################################################
sample <- read.table( "rr_samples.data", na.strings="None", header=TRUE)

# Crée formule
## Create a formula for a model with a large number of variables:
xnam <- paste("in_", 0:50, sep="")
ynam <- paste("ta_", 0:10, sep="")
fmla <- as.formula(paste( paste( ynam, collapse=" + "), " ~ ", paste(xnam, collapse=" + "), " - 1", sep=""))

## Ridge Regression
lm.ridge( fmla, data=leasample, lambda=0.1)
## Prépare autant d'interpolation que d'élément de ynam
rr.formulae <- lapply( ynam, function(x) as.formula(paste( x, " ~ ", paste(xnam, collapse=" + "), " - 1", sep="")))
## Appliquer ensuite lm.ridge
rr.results <- lapply( rr.formulae, function(x) lm.ridge( x, data=leasample, lambda=0.1))

## Essai de retrouver Y avec WX
## Façon C++, target 1
sum(le_w[1,] * leasample[1,1:51])
## Facon R, target 1
sum( res$coef * res$scales * leasample[1,1:51])




