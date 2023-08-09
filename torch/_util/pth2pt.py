import torch
import torchvision
import re
import torch
import os
import random
import sys

#from models.neural import *
from transformer_net import TransformerNet

tag = sys.argv[1]
if len(sys.argv) > 2:
	tag2 = tag + sys.argv[2]
	tag += ".pt"
else:
	tag += ".pt"
	tag2 = tag + "h"

style_model = TransformerNet()
state_dict = torch.load(tag2)
for k in list(state_dict.keys()):
    if re.search(r'in\d+\.running_(mean|var)$', k):
        del state_dict[k]
style_model.load_state_dict(state_dict)
style_model.eval()
        
# model = torchvision.models.resnet50(pretrained=True)
# model.eval()
example = torch.rand(1, 3, 224, 224)
traced_script_module = torch.jit.trace(style_model, example)
traced_script_module.save(tag) 
