import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
import firebase_admin
from firebase_admin import credentials, db
import os
from dotenv import load_dotenv

class FirebaseRosBridge(Node):
    def __init__(self):
        super().__init__('firebase_ros_bridge')
        self.get_logger().info('Initializing ROS 2 <-> Firebase RTDB Bridge...')
        
        # ROS 2 Local Publishers
        self.cmd_vel_pub = self.create_publisher(Twist, '/cmd_vel', 10)
        
        # Firebase Cloud Setup
        load_dotenv()
        cred = credentials.Certificate(os.getenv("FIREBASE_CREDENTIALS_PATH", "./serviceAccountKey.json"))
        firebase_admin.initialize_app(cred, {
            'databaseURL': os.getenv("FIREBASE_DATABASE_URL")
        })
        
        # Bind asynchronous listener to the cloud control node
        self.ref = db.reference('/control/joystick')
        self.ref.listen(self.joystick_callback)
        self.get_logger().info('Subscribed to Cloud RTDB /control/joystick stream.')

    def joystick_callback(self, event):
        """Asynchronous callback triggered remotely by Flutter UI or Edge AI"""
        if isinstance(event.data, dict):
            # Parse vectors from Cloud
            x = event.data.get('x', 0.0)
            y = event.data.get('y', 0.0)
            
            # Map to standard ROS 2 Twist message
            msg = Twist()
            msg.linear.x = float(y)  # In our schema, Y is forward
            msg.angular.z = float(x) # X is rotation/steering
            
            # Publish to local DDS network for Nav2 consumption
            self.cmd_vel_pub.publish(msg)
            self.get_logger().debug(f'Published /cmd_vel -> linear_x: {msg.linear.x}, angular_z: {msg.angular.z}')

def main(args=None):
    rclpy.init(args=args)
    bridge_node = FirebaseRosBridge()
    
    try:
        # Spin to keep the node active and listening to both DDS and Firebase threads
        rclpy.spin(bridge_node)
    except KeyboardInterrupt:
        pass
    finally:
        bridge_node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
