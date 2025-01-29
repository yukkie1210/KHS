import numpy as np
import math
import random

#順運動学の式を求める関数
def FK(L, js):
  Ts = []
  c0 = math.cos(js[0])
  s0 = math.sin(js[0])
  c1 = math.cos(js[1])
  s1 = math.sin(js[1])
  c2 = math.cos(js[2])
  s2 = math.sin(js[2])
  c3 = math.cos(js[3])
  s3 = math.sin(js[3])
  c4 = math.cos(js[4])
  s4 = math.sin(js[4])
  c5 = math.cos(js[5])
  s5 = math.sin(js[5])

  Ts.append(np.matrix([[c0, -s0, 0, 0], [s0, c0, 0, 0], [0, 0, 1, L[0]], [0, 0, 0, 1]]))
  Ts.append(np.matrix([[1, 0, 0, 0], [0, c1, -s1, -0.03], [0, s1, c1, L[1]], [0, 0, 0, 1]]))
  Ts.append(np.matrix([[1, 0, 0, 0], [0, c2, -s2, 0], [0, s2, c2, L[2]], [0, 0, 0, 1]]))
  Ts.append(np.matrix([[c3, -s3, 0, 0], [s3, c3, 0, 0], [0, 0, 1, L[3]], [0, 0, 0, 1]]))
  Ts.append(np.matrix([[1, 0, 0, 0], [0, c4, -s4, 0], [0, s4, c4, L[4]], [0, 0, 0, 1]]))
  Ts.append(np.matrix([[c5, -s5, 0, 0], [s5, c5, 0, 0], [0, 0, 1, L[5]], [0, 0, 0, 1]]))
  Ts.append(np.matrix([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, L[6]], [0, 0, 0, 1]]))

  return [Ts[0], Ts[0]*Ts[1], Ts[0]*Ts[1]*Ts[2], Ts[0]*Ts[1]*Ts[2]*Ts[3], Ts[0]*Ts[1]*Ts[2]*Ts[3]*Ts[4],
          Ts[0]*Ts[1]*Ts[2]*Ts[3]*Ts[4]*Ts[5], Ts[0]*Ts[1]*Ts[2]*Ts[3]*Ts[4]*Ts[5]*Ts[6]]

#クォータニオンを生成する関数
def make_q(L, theta):
  return [L[0]*math.sin(theta/2), L[1]*math.sin(theta/2), L[2]*math.sin(theta/2), math.cos(theta/2)]

#クォータニオンの積を計算する関数
def my_cross(q, p):
  return [q[3]*p[0] - q[2]*p[1] + q[1]*p[2] + q[0]*p[3],
          q[2]*p[0] + q[3]*p[1] - q[0]*p[2] + q[1]*p[3],
          -q[1]*p[0] + q[0]*p[1] + q[3]*p[2] + q[2]*p[3],
          -q[0]*p[0] - q[1]*p[1] - q[2]*p[2] + q[3]*p[3]]

#各関節角度から手先姿勢を求める関数
def check_q(Theta):
  s53_0 = math.sin(Theta[5]/2 + Theta[3]/2 - Theta[0]/2)
  c53_0 = math.cos(Theta[5]/2 + Theta[3]/2 - Theta[0]/2)
  s5_3_0 = math.sin(Theta[5]/2 - Theta[3]/2 - Theta[0]/2)
  c5_3_0 = math.cos(Theta[5]/2 - Theta[3]/2 - Theta[0]/2)
  s3_0_5 = math.sin(Theta[3]/2 - Theta[0]/2 - Theta[5]/2)
  c3_0_5 = math.cos(Theta[3]/2 - Theta[0]/2 - Theta[5]/2)
  s530 = math.sin(Theta[5]/2 + Theta[3]/2 + Theta[0]/2)
  c530 = math.cos(Theta[5]/2 + Theta[3]/2 + Theta[0]/2)
  s12 = math.sin(Theta[1]/2 + Theta[2]/2)
  c12 = math.cos(Theta[1]/2 + Theta[2]/2)
  s4 = math.sin(Theta[4]/2)
  c4 = math.cos(Theta[4]/2)

  return [c53_0*c4*s12 + c5_3_0*s4*c12,
          -s53_0*c4*s12 - s5_3_0*s4*c12,
          s3_0_5*s4*s12 + s530*c4*c12,
          -c3_0_5*s4*s12 + c530*c4*c12]

#θの範囲を[-π, π]に直す関数
def fixPi(Js):
  js = Js
  for j in range(len(Js)):
    if js[j] > math.pi:
      js[j] = js[j] - int((js[j] + math.pi)/(2*math.pi))*2*math.pi
    elif js[j] < -math.pi:
      js[j] = js[j] - int((js[j]-math.pi)/(2*math.pi))*2*math.pi
  return js

#手先位置[x, y, z], 手先姿勢quaternionから各関節角度jsを求める関数
def AnalyticSolution(x, y, z, quaternion):
  L = [6, 75.7, 151, 112.7, 75.7, 112.7, 117.3] #アームの長さ
  q = quaternion #手先姿勢[qx, qy, qz, qw]

  vx = 2*(q[0]*q[2] + q[3]*q[1])
  vy = 2*(q[1]*q[2] - q[3]*q[0])
  vz = q[3]**2 - q[0]**2 - q[1]**2 + q[2]**2

  x4 = x - (L[5] + L[6])*vx
  y4 = y - (L[5] + L[6])*vy
  z4 = z - (L[5] + L[6])*vz

  if x4==0 and y4==0:
    Theta0 = 0
  else:
    Theta0 = math.atan2(-x4, y4)

  x1 = 0.03*math.sin(Theta0)
  y1 = -0.03*math.cos(Theta0)
  z1 = L[0] + L[1]

  L14 = math.sqrt((x4 - x1)**2 + (y4 - y1)**2 + (z4 - (L[0] + L[1]))**2)
  Theta2 = math.acos(-(L[2]**2 + (L[3] + L[4])**2 - L14**2)/(2*L[2]*(L[3] + L[4])))

  Theta1_1 = math.acos((L[2]**2 + L14**2 - (L[3] + L[4])**2)/(2*L[2]*L14))
  L04_ = math.sqrt((x4 - x1)**2 + (y4 - y1)**2 + (z4 - L[0])**2)
  Theta1_2 = math.acos((L[1]**2 + L14**2 - L04_**2)/(2*L[1]*L14))
  Theta1 = math.pi - (Theta1_1 + Theta1_2)

  #θ0とθ1が分かれば[x2, y2, z2]が求まる
  xyz2 = FK(L, [Theta0, Theta1, 0, 0, 0, 0])
  x2 = xyz2[2][0, 3]
  y2 = xyz2[2][1, 3]
  z2 = xyz2[2][2, 3]

  L26_2 = (x - x2)**2 + (y - y2)**2 + (z - z2)**2 #L26の2乗
  Theta4 = math.acos(-((L[3] + L[4])**2 + (L[5] + L[6])**2 - L26_2)/(2*(L[3] + L[4])*(L[5] + L[6])))

  #関節角度, 回転軸からクォータニオンq0, q1, q2をつくる
  my_q = []
  my_q.append(make_q([0,0,1], Theta0))
  my_q.append(make_q([1,0,0], Theta1))
  my_q.append(make_q([1,0,0], Theta2))

  #クォータニオンq0, q1, q2の積を取りq02を得る
  my_Q = []
  my_Q.append(my_cross(my_q[0], my_q[1]))
  my_Q.append(my_cross(my_Q[0], my_q[2]))
  qx02 = my_Q[1][0]
  qy02 = my_Q[1][1]
  qz02 = my_Q[1][2]
  qw02 = my_Q[1][3]

  C4 = math.cos(Theta4)
  S4 = math.sin(Theta4)

  #方程式を解いてθ3を得る(方程式の導出過程は省略)
  A = np.matrix([[2*(qz02*qw02 - qx02*qy02)*S4, (qx02**2 - qy02**2 - qz02**2 + qw02**2)*S4],
                 [(qx02**2 - qy02**2 + qz02**2 - qw02**2)*S4, 2*(qw02*qz02 + qx02*qy02)*S4]])
  B = np.matrix([[vx - 2*(qz02*qx02 + qw02*qy02)*C4], [vy - 2*(qy02*qz02 - 
  qx02*qw02)*C4]])
  invA = np.linalg.inv(A)
  XY = invA*B
  Theta3 = math.atan2(XY[1], XY[0])

  #q3, q4をつくる
  my_q.append(make_q([0,0,1], Theta3))
  my_q.append(make_q([1,0,0], Theta4))

  #q04を得る
  my_Q.append(my_cross(my_Q[1], my_q[3]))
  my_Q.append(my_cross(my_Q[2], my_q[4]))
  qx04 = my_Q[3][0]
  qy04 = my_Q[3][1]
  qz04 = my_Q[3][2]
  qw04 = my_Q[3][3]

  #手先姿勢quaternion
  qx5 = q[0]
  qy5 = q[1]
  qz5 = q[2]
  qw5 = q[3]

  #方程式を解いてθ5を得る(方程式の導出過程は省略)
  C5 = (qx04*qx5 + qy04*qy5)/(qx04**2 + qy04**2)
  S5 = (qx5*qy04 - qy5*qx04)/(qx04**2 + qy04**2)
  Theta5 = 2*math.atan2(S5, C5)

  #各関節角度js
  js = fixPi([Theta0, Theta1, Theta2, Theta3, Theta4, Theta5])

  #逆運動学を解いて出した手先姿勢check_qtn
  check_qtn = check_q(js)

  if quaternion[0]*check_qtn[0] > 0: #手先姿勢が正しい値と一致した場合
    return js
  else:  #手先姿勢が一致しない場合
    Theta4_ = -Theta4

    qx02 = my_Q[1][0]
    qy02 = my_Q[1][1]
    qz02 = my_Q[1][2]
    qw02 = my_Q[1][3]
    C4_ = math.cos(Theta4_)
    S4_ = math.sin(Theta4_)

    A_ = np.matrix([[2*(qz02*qw02 - qx02*qy02)*S4_, (qx02**2 - qy02**2 - qz02**2 + qw02**2)*S4_],
                [(qx02**2 - qy02**2 + qz02**2 - qw02**2)*S4_, 2*(qw02*qz02 + qx02*qy02)*S4_]])
    B_ = np.matrix([[vx - 2*(qz02*qx02 + qw02*qy02)*C4_], [vy - 2*(qy02*qz02 - qx02*qw02)*C4_]])
    invA_ = np.linalg.inv(A_)
    XY_ = invA_*B_
    Theta3_ = math.atan2(XY_[1], XY_[0])

    my_q_ = []
    my_q_.append(make_q([0, 0, 1], Theta0))
    my_q_.append(make_q([1, 0, 0], Theta1))
    my_q_.append(make_q([1, 0, 0], Theta2))
    my_q_.append(make_q([0, 0, 1], Theta3_))
    my_q_.append(make_q([1, 0, 0], Theta4_))

    my_Q_ = []
    my_Q_.append(my_cross(my_q_[0], my_q_[1]))
    my_Q_.append(my_cross(my_Q_[0], my_q_[2]))
    my_Q_.append(my_cross(my_Q_[1], my_q_[3]))
    my_Q_.append(my_cross(my_Q_[2], my_q_[4]))

    qx04 = my_Q_[3][0]
    qy04 = my_Q_[3][1]
    qz04 = my_Q_[3][2]
    qw04 = my_Q_[3][3]
    qx5 = q[0]
    qy5 = q[1]
    qz5 = q[2]
    qw5 = q[3]

    C5_ = (qx04*qx5 + qy04*qy5)/(qx04**2 + qy04**2)
    S5_ = (qx5*qy04 - qy5*qx04)/(qx04**2 + qy04**2)
    Theta5_ = 2*math.atan2(S5_, C5_)

    js = fixPi([Theta0, Theta1, Theta2, Theta3_, Theta4_, Theta5_])
    return js



L = [6, 75.7, 151, 112.7, 75.7, 112.7, 117.3]
  
random_js = []
random_js.append(random.uniform(-math.pi*1/2, math.pi*1/2))
random_js.append(random.uniform(-math.pi*1/2, math.pi*1/2))
random_js.append(random.uniform(-math.pi*1/2, math.pi*1/2))
random_js.append(random.uniform(-math.pi*1/2, math.pi*1/2))
random_js.append(random.uniform(-math.pi*1/2, math.pi*1/2))
random_js.append(random.uniform(-math.pi*1/2, math.pi*1/2))
random_js = fixPi(random_js)

#random_jsから、手先座標[x, y, z], 姿勢qtnを導出
TTs = FK(L, random_js)
qtn = check_q(random_js)
x = TTs[6][0, 3]
y = TTs[6][1, 3]
z = TTs[6][2, 3]

#逆運動学で各関節角度calc_jsを求める
calc_js = AnalyticSolution(x, y, z, qtn)

#calc_jsから、手先座標[calc_x, calc_y, calc_z], 姿勢calc_qtnを導出
calc_TTs = FK(L, calc_js)
calc_qtn = check_q(calc_js)
calc_x = calc_TTs[6][0, 3]
calc_y = calc_TTs[6][1, 3]
calc_z = calc_TTs[6][2, 3]

print(random_js)
print("     xyz:", [x, y, z])
print("      Quaternion:", qtn)

print("calc_xyz:", [calc_x, calc_y, calc_z])
print(" calc_Quaternion:", calc_qtn)