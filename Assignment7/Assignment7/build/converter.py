from PIL import Image
img = Image.open('binary.ppm')
img.save('ray_tracing_result.png')