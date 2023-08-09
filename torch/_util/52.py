import time
import cv2
import torch 
import argparse
import numpy as np
import os
import torch.nn.functional as F
torch.cuda.set_device(0)

path = '/zzzzzzzzzzz6/zzzzzzzzzzz4/opt/src/github.com.cnpmjs.org/BITHG287/Semantic_Human_Matting-PyTorch_OpenVINO'
parser = argparse.ArgumentParser(description='human matting')
parser.add_argument('--model', default=path + '/ckpt/human_matting/model/model_obj.pth', help='preTrained model')
parser.add_argument('--img', '-i', default=path + '/__images')
parser.add_argument('--img2', '-i2', default=path + '/_images')
parser.add_argument('--size', type=int, default=256, help='input size')
parser.add_argument('--without_gpu', action='store_true', default=False, help='no use gpu')
parser.add_argument('--and-bg', '-a', action='store_false', default=True)
parser.add_argument('--show', '-s', action='store_true', default=False)
parser.add_argument('--v', default=75, type=float, help='侵剥度，这是一根刻度0到100的拖动条')
parser.add_argument('--save-mod', '-m2')
parser.add_argument('--help2', '-h2', action='store_true', default=False)

args = parser.parse_args()
torch.set_grad_enabled(False)

args.v = (100 - args.v) / 100
if args.save_mod:
    args.without_gpu = True

if args.without_gpu:
    print("use CPU !")
    device = torch.device('cpu')
else:
    if torch.cuda.is_available():
        n_gpu = torch.cuda.device_count()
        print("----------------------------------------------------------")
        print("|       use GPU !      ||   Available GPU number is {} !  |".format(n_gpu))
        print("----------------------------------------------------------")

        device = torch.device('cuda:0')

def load_model():
    print('Loading model from {}...'.format(args.model))
    if args.without_gpu:
        myModel = torch.load(args.model, map_location=lambda storage, loc: storage)
    else:
        myModel = torch.load(args.model)

    myModel.eval()
    myModel.to(device)
    
    return myModel


def seg_process(image, net):

    origin_h, origin_w, c = image.shape
    image_resize = cv2.resize(image, (args.size,args.size), cv2.INTER_CUBIC)
    # print(image_resize.shape)

    image_resize = (image_resize - (104., 112., 121.,)) / 255.0
    tensor_4D = torch.FloatTensor(1, 3, args.size, args.size)
    tensor_4D[0,:,:,:] = torch.FloatTensor(image_resize.transpose(2,0,1))
    inputs = tensor_4D.to(device)

    t0 = time.time()
    trimap, alpha = net(inputs)
    print('infer cost time:', (time.time() - t0))  

    if args.without_gpu:
        alpha_np = alpha[0,0,:,:].data.numpy()
        trimap_np = trimap[0,0,:,:].data.numpy()
    else:
        alpha_np = alpha[0,0,:,:].cpu().data.numpy()
        trimap_np = trimap[0,0,:,:].cpu().data.numpy()

    alpha_np = cv2.resize(alpha_np, (origin_w, origin_h), cv2.INTER_CUBIC)
    fg = np.multiply(alpha_np[..., np.newaxis].astype(np.float32), image.astype(np.float32))
    if args.help2:
        #cv2.imwrite("/tmp/52-alpha.png", alpha_np)
        print(fg.shape, 'fg')
        cv2.imwrite("/tmp/52-fg.png", fg)
    out = None

    if args.and_bg:
        bg = image
        bg_gray = np.multiply(1-alpha_np[..., np.newaxis], image)
        bg_gray = cv2.cvtColor(bg_gray, cv2.COLOR_BGR2GRAY)

        bg[:,:,0] = bg_gray
        bg[:,:,1] = bg_gray
        bg[:,:,2] = bg_gray

        out = fg + bg
    else:
        b_channel, g_channel, r_channel = cv2.split(fg)
        alpha_channel = np.ones(b_channel.shape, dtype=b_channel.dtype) * 255

        bg = np.multiply(1-alpha_np[..., np.newaxis], 1)

        if args.help2:
            print(bg.shape, "bg0")
            cv2.imwrite("/tmp/52-bg0.png", bg)
            print(b_channel.shape, 'b_channel')
            cv2.imwrite("/tmp/52-fg-b.png", b_channel)
            cv2.imwrite("/tmp/52-fg-g.png", g_channel)
            cv2.imwrite("/tmp/52-fg-r.png", r_channel)
            bg2 = np.multiply(1-alpha_np[..., np.newaxis], image)
            cv2.imwrite("/tmp/52-bg.png", bg2)

        #alpha_channel[bg] = 0
        cnt = cnt2 = cnt3 = cnt4 = cnt5 = cnt6 = cnt7 = 0
        for index, value in enumerate(bg):
            for index2, value2 in enumerate(value):
                if value2 > args.v:
                    alpha_channel[index][index2] = 0
                cnt += 1
                if value2 >= 1:
                    cnt2 += 1
                if value2 >= 0.9:
                    cnt3 += 1
                if value2 >= 0.5:
                    cnt4 += 1
                if value2 >= 0.25:
                    cnt5 += 1
                if value2 >= 0.0001:
                    cnt6 += 1
                if value2 <= 0:
                    cnt7 += 1
        print("cnt:", cnt, cnt2, cnt3, cnt4, cnt5, cnt6, cnt7)

        out = cv2.merge((b_channel, g_channel, r_channel, alpha_channel))

        #bg = np.multiply(1-alpha_np[..., np.newaxis], np.array([0,255,0]).astype(np.float32))
        #out = cv2.addWeighted(fg, 1, bg,1, 0)

    out[out<0] = 0
    out[out>255] = 255
    out = out.astype(np.uint8)

    return out

def z__(img_path, net):
    img = cv2.imread(img_path)
    img_seg = seg_process(img, net)

    png = None
    if os.path.isdir(args.img2):
        ext = os.path.split(img_path)
        ext = os.path.splitext(ext[1])
        png = args.img2 + "/" + ext[0] + ".png"
    else:
        png = args.img2
    print(png)
    cv2.imwrite(png, img_seg)

    if args.show:
        cv2.imshow("img", img)
        cv2.imshow("img_seg", img_seg)
        cv2.waitKey(0)

def camera_seg(net):
    if args.show:
        cv2.namedWindow("img", 0)
        cv2.namedWindow("img_seg", 0)
    if os.path.isdir(args.img):
        for f in os.listdir(args.img):
            z__(os.path.join(args.img, f), net)
    else:
        z__(args.img, net)

    if args.show:
        cv2.destroyAllWindows()


def main():
    myModel = load_model()
    if args.save_mod:
        example = torch.rand(1, 3, 320, 480)#生成一个随机输入维度的输入
        traced_script_module = torch.jit.trace(myModel, example)
        traced_script_module.save(args.save_mod)
        return
    camera_seg(myModel)

if __name__ == "__main__":
    main()
