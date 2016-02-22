# utilise ggplot
require(ggplot2)

# Lit les résultats
res <- read.table(file= "result", header=TRUE)
# ajoute la position des max
res$ta_idx <- apply(res[,1:10], 1, which.max)
res$le_idx <- apply(res[,11:20], 1, which.max)
res$in_idx <- apply(res[,21:30], 1, which.max)
# teste le nb de bonne réponses
sum(res$ta_idx == res$le_idx)
sum(res$ta_idx == res$in_idx)
# vecteur d'indices de 1 à 10
rg <- seq(1,10)
# Calcule la somme des carrés de l'erreur entre ta et le
mse1 <- function(row) {sum((row[rg] - row[rg+10])*(row[rg] - row[rg+10]))}
# Calcule la somme des carrés de l'erreur entre ta et in
mse2 <- function(row) {sum((row[rg] - row[rg+20])*(row[rg] - row[rg+20]))}
# ajoute dans la dataframe
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
  else if(target == learn) {
    return ("init")
  }
  else {
    return ("none")
  }
}
# labellise les données
res$label <- mapply(label_learn,res$ta_idx, res$le_idx, res$in_idx)


# plot Erreur en LEARN vs Erreur en INIT
# Ce qui est au dessus de la diagonale est mieux appris par LEARN
# TODO Couleur en fonction de si le résultat est bon.
# root layer
p_root <- ggplot(res)
# Chacune des lignes, avec la couleur qui dépend de son nom
p_scatter <- geom_point( aes( x=le_mse, y=in_mse, color=label))
# ligne diagonale
p_lin <- geom_abline( slope=1, intercept=0)
# Choix manuel des couleurs utilisées en vrai (ordre est important)
# p_col <- scale_color_manual( values=c('red','green'))
# Titre, axes et nom de la legende de couleur
p_labs <- labs(colour=NULL, title="LEARN vs INIT", x="le", y="in")
# Put upper-right corner of legend box in upper-right corner of graph
# p_leg <- theme(legend.justification=c(1,1), legend.position=c(1,1))
# AU FINAL
p_root + p_scatter + p_lin + p_labs


