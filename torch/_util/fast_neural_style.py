#!/usr/bin/python3
import torch
from torchvision import transforms
# ln -s /opt2/src/github.com/pytorch/examples/fast_neural_style/neural_style/transformer_net.py
from transformer_net import TransformerNet
import re

from PIL import Image

def saveimg(content, path):
    img = content.clone().clamp(0, 255).numpy()
    img = img.transpose(1, 2, 0).astype("uint8")
    img = Image.fromarray(img)
    img.save(path)

def stylize(args):
    device = torch.device("cuda" if args.cuda else "cpu")
    print(device)

    content_image = Image.open(args.content_image)
    content_transform = transforms.Compose([
        transforms.ToTensor()
        ,
        transforms.Lambda(lambda x: x.mul(255))
    ])
    content_image = content_transform(content_image)
    saveimg(content_image, args.output_image_1 + "2" + args.output_image_2)
    content_image = content_image.unsqueeze(0).to(device)

    with torch.no_grad():
        style_model = TransformerNet()
        state_dict = torch.load(args.model)
        # remove saved deprecated running_* keys in InstanceNorm from the checkpoint
        for k in list(state_dict.keys()):
            if re.search(r'in\d+\.running_(mean|var)$', k):
                del state_dict[k]
        style_model.load_state_dict(state_dict)
        style_model.to(device)
        output = style_model(content_image).cpu()

    saveimg(output[0], args.output_image_1 + args.output_image_2)

class args__:
	def __init__(self):
		self.cuda = False
		self.cuda = torch.cuda.is_available()
		print(self.cuda)
		self.content_image = "/opt2/src/github.com/pytorch/examples/fast_neural_style/images/content-images/amber.jpg"
		#self.model = "/opt2/src/github.com/pytorch/examples/fast_neural_style/saved_models/candy.pth"
		self.model = "saved_models/candy.pth"
		self.output_image_1 = "/tmp/out"
		self.output_image_2 = ".png"

def main():
	stylize(args__())

if __name__ == "__main__":
    main()
