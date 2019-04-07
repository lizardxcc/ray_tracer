import bpy
import sys
import math
sys.path.append('./')
from bin.release import renderer

print(sys.version_info)

scene = bpy.data.scenes["Scene"]
camera = scene.camera
cam = bpy.data.cameras[scene.camera.name]

renderer.set_camera(camera.location[0], camera.location[2], -camera.location[1],
camera.rotation_axis_angle[1], camera.rotation_axis_angle[3], -camera.rotation_axis_angle[2], camera.rotation_axis_angle[0],
1.0*math.degrees(cam.angle)/scene.render.resolution_x*scene.render.resolution_y,
1.0*scene.render.resolution_x/scene.render.resolution_y
)
renderer.execute(scene.render.resolution_x, scene.render.resolution_y, scene.cycles.samples, "test.pnm")
