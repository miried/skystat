####################################
# skyplot script
####################################

# all
allallA = scan( "/dev/shm/skyplot/allall-NVSSA.d")
allallA_nn = scan( "/dev/shm/skyplot/allall-NVSSA.e")

allsfA = scan( "/dev/shm/skyplot/allsf-NVSSA.d")
allsfA_nn = scan( "/dev/shm/skyplot/allsf-NVSSA.e")

allnsfA = scan( "/dev/shm/skyplot/allnsf-NVSSA.d")
allnsfA_nn = scan( "/dev/shm/skyplot/allnsf-NVSSA.e")

allB = scan( "/dev/shm/skyplot/all-NVSSB.d")
allB_nn = scan( "/dev/shm/skyplot/all-NVSSB.e")

allsf = ks.test(abs(allsfA),abs(allB))
allsf_nn = ks.test(abs(allsfA_nn),abs(allB_nn))

allnsf = ks.test(abs(allnsfA),abs(allB))
allnsf_nn = ks.test(abs(allnsfA_nn),abs(allB_nn))

# corr
corrsfA = scan( "/dev/shm/skyplot/corrsf-NVSSA.d")
corrsfA_nn = scan( "/dev/shm/skyplot/corrsf-NVSSA.e")

corrnsfA = scan( "/dev/shm/skyplot/corrnsf-NVSSA.d")
corrnsfA_nn = scan( "/dev/shm/skyplot/corrnsf-NVSSA.e")

corrB = scan( "/dev/shm/skyplot/corr-NVSSB.d")
corrB_nn = scan( "/dev/shm/skyplot/corr-NVSSB.e")

corrsf = ks.test(abs(corrsfA),abs(corrB))
corrsf_nn = ks.test(abs(corrsfA_nn),abs(corrB_nn))

corrnsf = ks.test(abs(corrnsfA),abs(corrB))
corrnsf_nn = ks.test(abs(corrnsfA_nn),abs(corrB_nn))

# lat
latsfA = scan( "/dev/shm/skyplot/latsf-NVSSA.d")
latsfA_nn = scan( "/dev/shm/skyplot/latsf-NVSSA.e")

latnsfA = scan( "/dev/shm/skyplot/latnsf-NVSSA.d")
latnsfA_nn = scan( "/dev/shm/skyplot/latnsf-NVSSA.e")

latB = scan( "/dev/shm/skyplot/lat-NVSSB.d")
latB_nn = scan( "/dev/shm/skyplot/lat-NVSSB.e")

latsf = ks.test(abs(latsfA),abs(latB))
latsf_nn = ks.test(abs(latsfA_nn),abs(latB_nn))

latnsf = ks.test(abs(latnsfA),abs(latB))
latnsf_nn = ks.test(abs(latnsfA_nn),abs(latB_nn))

ma = array(c(allsf$p.value,corrsf$p.value,latsf$p.value,
allsf_nn$p.value,corrsf_nn$p.value,latsf_nn$p.value,
0,0,0,
allnsf$p.value,corrnsf$p.value,latnsf$p.value,
allnsf_nn$p.value,corrnsf_nn$p.value,latnsf_nn$p.value
),dim=c(3,5))

print(ma)
