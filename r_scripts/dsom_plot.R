# utilise ggplot
require(ggplot2)
require(rjson)

dno <- read.table( file="normal.data", sep="\t")
names(dno) = c("x","y")

## afficher un plot
p_root <- ggplot()
p_nor <- geom_point( data=dno, aes(x=x, y=y), size=1, colour="blue")
p_root + p_nor

# ******************************************************************** TODO
## afficher un DSOM-2D
jdata <- fromJSON( file="dsom.data" )
rbind( jdata$neurons[[1]]$pos, jdata$neurons[[2]]$pos, jdata$neurons[[3]]$pos)

## matrix of positions from json data of network
mk_pos <- function( data_json ) 
{
  mpos <- rbind( data_json$neurons[[1]]$pos )
  for( i in seq(from=2,to=length(data_json$neurons)))
    mpos <- rbind( mpos, data_json$neurons[[i]]$pos)
  ##df.pos <- data.frame(df.pos)
  ##names( df.pos ) <- c( "x", "y" )
  
  return (mpos)
}
mk_wpos <- function( data_json ) 
{
  mpos <- rbind( data_json$neurons[[1]]$weights )
  for( i in seq(from=2,to=length(data_json$neurons)))
    mpos <- rbind( mpos, data_json$neurons[[i]]$weights)
  ##df.pos <- data.frame(df.pos)
  ##names( df.pos ) <- c( "x", "y" )
  
  return (mpos)
}

mk_links <- function( allpos, neuron )
{
  mk_seg <- function(x)
  {
    return (c(allpos[neuron$id+1,], allpos[x+1,]))
  }
  res <- sapply( neuron$link, FUN=mk_seg )
  res <- t(res) ## transpose the matrix
  return (res)
}
mk_all_link <- function( allpos, l_neurons )
{
  ## links init
  mylinks <- mk_links( allpos, l_neurons[[1]] )
  ## add other links
  for( i in seq(from=2,to=length(l_neurons)-1))
    mylinks <- rbind( mylinks, mk_links(allpos, l_neurons[[i]] ))
  
  return (mylinks)
}
##
## ggplot() + geom_segment( data=sgments, aes(x=x,y=y,xend=xend,yend=yend)) + geom_point( data=mypts, aes(x=x,y=y), shape = 21, colour = "black", fill = "white", size=4, stroke=2)

## return ggplot elements (segment+points) to display a network.
## gnet <- gplot_network( jdata )
## ggplot() + gnet[1] + gnet[2]
gplot_network <- function( data_json )
{
  #compute the weight_positions of the neurons
  wpos <- mk_wpos( data_json )
  # compute the links
  wlink <- mk_all_link( wpos, data_json$neurons)
  # as data.frames
  wpos <- as.data.frame( wpos )
  names(wpos) <- c("x","y")
  wlink <- as.data.frame( wlink )
  names(wlink) <- c("x","y","xend","yend")
  
  return (c(geom_segment( data=wlink, aes(x=x,y=y,xend=xend,yend=yend)),
          geom_point( data=wpos, aes(x=x,y=y), shape = 21, colour = "black", fill = "white", size=4, stroke=2)))
}