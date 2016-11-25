## saving session
## p05ABCB - 24/11/2016
source('~/Projets/VisuGL/r_scripts/hmm_stat.R')

df <- read.table( file="data_hmm/df_p05ABCB.rdata", sep="\t", header=TRUE)
summary( df )

# keep only test in dfT
attach(df)
dfT <- df[type=="test",]
detach(df)

# ordered indices accordingto precision
attach(dfT)
idx_bestp <- order( prec_err, decreasing = TRUE  )
n <- length(idx_bestp)
detach(dfT)

# plot 10 best, 10 worst
plot_many_traj( dfT, idx_bestp[1:10], "data_hmm/res_p05ABCB", "Pic_tmp/")
plot_many_traj( dfT, idx_bestp[(n-9):n], "data_hmm/res_p05ABCB", "Pic_tmp/")

# Compute mean
attach(dfT)
dfT.mean <- aggregate(dfT[,c("prec_err", "mse_err")], by=list(ltraj,lesn,leak,regul,fw,lnoise,ltest), FUN=mean )
names(dfT.mean)[1:7] <- c("ltraj","lesn","leak","regul","fw","lnoise","ltest")
detach(dfT)
# and order by prec
attach(dfT.mean)
idx_bestm <- order( prec_err, decreasing= TRUE )
dfT.mean[idx_bestm[1:25],]
detach(dfT.mean)

