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
