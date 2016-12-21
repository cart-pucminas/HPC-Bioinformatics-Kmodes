    #!/bin/bash

# for i in {1..10}
# do
# 	diff ../input/gallus/cluster$i  ../input_cuda/gallus/cluster$i
# 	diff 	../input/Rattusnovergicus/cluster$i 	../input_cuda/Rattusnovergicus/cluster$i
# 	diff 	../input/celegans/cluster$i 	../input_cuda/celegans/cluster$i
# 	diff 	../input/athaliana/cluster$i	../input_cuda/athaliana/cluster$i
# 	diff 	../input/Drosophila/cluster$i	../input_cuda/Drosophila/cluster$i
# 	diff 	../input/HomoSapiens/cluster$i	../input_cuda/HomoSapiens/cluster$i
# done

for i in {1..10}
do
 	diff ../input/gallus/cluster$i.out  ../input_cuda/gallus/cluster$i.out
 	diff 	../input/Rattusnovergicus/cluster$i.out 	../input_cuda/Rattusnovergicus/cluster$i.out
 	diff 	../input/Musmusculus/cluster$i.out 	../input_cuda/Musmusculus/cluster$i.out
	diff 	../input/celegans/cluster$i.out 	../input_cuda/celegans/cluster$i.out
 	diff 	../input/athaliana/cluster$i.out 	../input_cuda/athaliana/cluster$i.out
# 	diff 	../input/Drosophila/cluster$i.out 	../input_cuda/Drosophila/cluster$i.out
# 	diff 	../input/HomoSapiens/cluster$i.out 	../input_cuda/HomoSapiens/cluster$i.out
done