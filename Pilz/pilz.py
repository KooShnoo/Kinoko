import importlib.util
from enum import IntFlag
import mathutils
import fake_bpy as bpy

pilz = importlib.util.module_from_spec(importlib.util.spec_from_file_location("pilz","/Users/ishanchawla/Desktop/Wii/dev/Kinoko/out/kinoko.so"))

class Button(IntFlag):
    ACCELERATE = 0x1
    BRAKE = 0x2
    ITEM = 0x4
    DRIFT = 0x8

key_to_button = {
    'W': Button.ACCELERATE,
    'SPACE': Button.BRAKE | Button.DRIFT,
    'LEFT_SHIFT': Button.ITEM,
}

class DriveCourse(bpy.types.Operator):
    bl_idname = "kooshnoo.drive"
    bl_label = "Test-Drive Course"
    buttons = Button(0)
    left = False
    right = False
    view_3d = None
    timer = None

    def __init__(self):
        print("Start")

    def __del__(self):
        print("End")
        
    def execute(self, context):
        return {'FINISHED'}

    def modal(self, context, event):
        if event.type == 'TIMER':
            self.drive_loop()
        elif event.type in {'RIGHTMOUSE', 'ESC'}:  # Cancel
            pilz.stop()
            # bpy.app.timers.unregister(self.drive_loop)
            # context.window_manager.event_timer_remove(self.timer)
            # return {'FINISHED'}
            return {'CANCELLED'}
        self.inputs(event)

        return {'RUNNING_MODAL'}

    def drive_loop(self):
        (pos, rot) = pilz.read_state()
        # scale down
        pos = tuple(map(lambda x: x / 1000, pos))
        pos = mathutils.Vector(pos)
        rot = mathutils.Matrix(rot)
        rot = rot.to_quaternion()

        # rot_adjust = mathutils.Euler((math.radians(-90.0), 0.0,math.radians(180.0)), 'XYZ')
        # rot.rotate(rot_adjust)
        bpy.context.active_object.location = pos
        bpy.context.active_object.rotation_quaternion = rot
        
        mepos = rot @ mathutils.Vector((0, 0, -1))
        mepos *= 1.2
        behind = pos + mepos
        behind.z -= 0.2
        

        facing = pos - behind
        facing = facing.to_track_quat('Z', 'Y').to_euler()
        facing = facing.to_quaternion()
        
        facing = mathutils.Euler((facing.to_euler().x, 0, facing.to_euler().z), 'XYZ').to_quaternion()

        self.view_3d.region_3d.view_location = pos
        self.view_3d.region_3d.view_rotation = facing
        return 1 / 15
    
    def inputs(self, event):
        button = key_to_button.get(event.type)
        if button is not None:
            if event.value == 'PRESS':
                self.buttons |= button
            elif event.value == 'RELEASE':
                self.buttons &= ~button
        elif event.type == 'A':
            if event.value == 'PRESS':
                self.left = True
            elif event.value == 'RELEASE':
                self.left = False
        elif event.type == 'D':
            if event.value == 'PRESS':
                self.right = True
            elif event.value == 'RELEASE':
                self.right = False
        else:
            return # not a relevant event
        neutral = self.left == self.right
        if neutral:
            stickX = 0
        elif self.left:
            stickX = -1
        elif self.right:
            stickX = 1
        pilz.set_buttons(self.buttons.value, stickX)
        

    def invoke(self, context, event):
        pilz.provide_files("/Users/ishanchawla/Desktop/Wii/dev/Kinoko/out/Race/common.szs","/Users/ishanchawla/Downloads/untitled.szs")
        pilz.set_course(0)
        pilz.init()
        pilz.start()
        bpy.app.timers.register(self.drive_loop)
        # self.timer = context.window_manager.event_timer_add(1 / 60, window=context.window)
        context.window_manager.modal_handler_add(self)
        for area in bpy.context.screen.areas:
            if area.type == 'VIEW_3D':
                for space in area.spaces:
                    if space.type == 'VIEW_3D':
                        self.view_3d = space

        # self.execute(context)
        return {'RUNNING_MODAL'}
    # def cancel():
        
    
# bpy_extras.keyconfig_utils.addon_keymap_register(keymap_data)

# Only needed if you want to add into a dynamic menu.
def menu_func(self, context):
    self.layout.operator(DriveCourse.bl_idname, text="Test-Drive Course")


# Register and add to the view menu (required to also use F3 search "Hello World Operator" for quick access).
bpy.utils.register_class(DriveCourse)
bpy.types.VIEW3D_MT_view.append(menu_func)