#!/bin/bash

source /opt/intel/bin/compilervars.sh intel64



# python executar_cuda.py ../input/gallus/ 12 9 ../input_cuda/gallus/
# python executar_cuda.py ../input/Rattusnovergicus/ 28 27 ../input_cuda/Rattusnovergicus/
# python executar_cuda.py ../input/Musmusculus/ 243 234 ../input_cuda/Musmusculus/
# python executar_cuda.py ../input/celegans/ 7887 7884 ../input_cuda/celegans/
# python executar_cuda.py ../input/athaliana/ 21096 21897 ../input_cuda/athaliana/
#python executar_cuda.py ../input/Drosophila/ 17804 17802 ../input_cuda/Drosophila/
python executar_cuda.py ../input/HomoSapiens/ 14261 14256 ../input_cuda/HomoSapiens/
