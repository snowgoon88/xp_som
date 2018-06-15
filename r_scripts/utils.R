
###############################################################################
## utils.R
##
## Some utils fonctions for R
## - multiplot(..., plotlist=NULL, file, cols=1, layout=NULL)
##      => plot multiple function in ggplot
## - to_pdf( filename, gplot )
##      => save ggplot to pdf
## - plot_adapt <- function( plot, title=NA, x=NA, y=NA, textsize=1, leg_rel=0.75, leg_pos=NULL)
##      => adapt text, legend and axis of plot
###############################################################################

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

###############################################################################
## plot in a pdf
## - filename : path of file, ending with .pdf
## - gplot : gplot command
##
to_pdf <- function( filename, gplot )
{
  # Must use print( cmd...) otherwise, nothing is plotted in script mode
  # Defaut size is 7x7 inches
  pdf( file=filename )
  print( gplot )
  dev.off()
}
###############################################################################

###############################################################################
## modify text size, title, axis and legend position
## - plot : a ggplot
## - title : if any, string for plot title
## - x : if any, string for x axis title
## - y : if any, string for y axis title
## - textsize : relative change in overall text size
## - leg_rel : size of legend relative to other text
## - leg_pos : c(x,y) if any, position of legend INSIDE plot (x,y \in [O,1])
##
## => ggplot
##
## Exampe : plot_adapt(pl, title="p05BCDEDC", x="time", y="weight", leg_pos=c(0.9,0.1), textsize = 1.5, leg_rel=1.5)
##
plot_adapt <- function( plot, title=NA, x=NA, y=NA, textsize=1, leg_rel=0.75, leg_pos=NULL)
{
  axis.size <- element_text( size = rel(textsize))
  tick.size <- element_text( size = rel(textsize*0.75))
  leg.size <- element_text( size = rel(textsize*leg_rel))
  leg.text.size <- element_text( size = rel(textsize*leg_rel*0.75))
  
  newtheme <- NULL
  if (is.null(leg_pos)) {
    newtheme <- theme( plot.title=element_text(size=rel(textsize+1), hjust=0.5),
                       axis.text=tick.size, axis.title=axis.size,
                       legend.title=leg.size, legend.text = leg.text.size)
  }
  else {
    newtheme <- theme( plot.title=element_text(size=rel(textsize+1), hjust=0.5),
                       axis.text=tick.size, axis.title=axis.size,
                       legend.title=leg.size, legend.text = leg.text.size,
                       legend.position = leg_pos )
  }
  if (!is.na(title)) {
    plot <- plot + ggtitle( title )
  }
  if (!is.na(x)) {
    plot <- plot + xlab( x )
  }
  if (!is.na(y)) {
    plot <- plot + ylab( y )
  }
  
  plot <- plot + newtheme
  return (plot)
}
###############################################################################
