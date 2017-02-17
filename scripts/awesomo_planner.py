#!/usr/bin/env python3
import itertools

from math import cos
from math import sin
from math import asin

import numpy as np
import scipy.optimize
import matplotlib.pylab as plt


def frange(x, y, jump, digits=0):
    array = []
    while x < y:
        array.append(round(x, digits))
        x += jump

    return array


def quadrotor(x, u, dt, cdx=0.5, cdz=0.2):
    g = 10.0
    h_ge = 1.0
    k_ge = 0.2
    a_ge = k_ge * (h_ge - min(h_ge, x[2])) / h_ge

    x[0] += x[1] * dt
    x[1] += (u[0] * sin(u[1]) - cdx * x[1]) * dt
    x[2] += x[3] * dt
    x[3] += (u[0] * cos(u[1]) * (1 + a_ge) - g - cdz * x[3]) * dt

    return x


def desired_system(p0_z, pf_z, v, dt, vz_max=-1.0):
    desired = []
    traj = []

    t_end = ((pf_z - p0_z) / vz_max)
    T = int(t_end / dt)

    p0 = [0.0, p0_z]
    pf = [(v * t_end), 0.0]

    # calculate line equation, gradient and intersect
    dx = (pf[0] - p0[0])
    dz = (pf[1] - p0[1])
    if dx != 0.0:
        m = dz / dx
    else:
        m = p0[0]
    c = p0[1] - m * p0[0]

    # calculate desired velocity and inputs
    vx = (pf[0] - p0[0]) / (T * dt)
    vz = (pf[1] - p0[1]) / (T * dt)
    az = 10.0
    theta = asin(vx / az)

    # initial point
    traj.append(p0)
    desired.append([p0[0], vx, p0[1], vz, az, theta])

    # create points along the desired line path
    dx = (pf[0] - p0[0]) / (T - 1.0)
    for i in range(1, T - 1):
        x_prev = desired[-1]

        x = [0 for i in range(6)]
        x[0] = x_prev[0] + dx      # x
        x[1] = vx                  # vx
        x[2] = m * x[0] + c        # z
        x[3] = vz                  # vz
        x[4] = az                  # az
        x[5] = theta               # theta

        desired.append(x)
        traj.append([x[0], x[2]])

    # final point
    traj.append(pf)
    desired.append([pf[0], 0.0, pf[1], 0.0, 0.0, 0.0])

    return (np.array(desired), np.array(traj), T)


def generate_bounds(T, n, m, nbs, mbs):
    x = []
    for i in range(0, T):
        # state bounds
        for j in range(n):
            x.append(nbs[j])

        # input bounds
        for k in range(m):
            x.append(mbs[k])

    return tuple(x)


def plot_desired_trajectory(traj):
    traj = traj.T
    plt.plot(traj[0], traj[1])
    plt.xlabel("x")
    plt.ylabel("z")
    plt.show()


def plot_optimization_results(traj, x, T, n, m):
    # setup
    x = x.reshape(T, n + m).T
    traj = traj.T
    dt = 0.1
    t = [i * dt for i in range(T)]

    plt.figure(1)

    plt.subplot(411)
    plt.tight_layout()

    plt.plot(traj[0], traj[1], label="desired")
    plt.plot(x[0], x[2], label="optimized")
    plt.title("Landing Trajectory")
    plt.legend(loc=0)
    plt.xlabel("x")
    plt.ylabel("z")

    plt.subplot(412)
    plt.plot(t, x[1], label="vx")
    plt.plot(t, x[3], label="vz")
    plt.title("Velocity")
    plt.legend(loc=0)
    plt.xlabel("t")
    plt.ylabel("v")

    plt.subplot(413)
    plt.title("Theta Input")
    plt.plot(t, x[5], label="theta")
    plt.legend(loc=0)
    plt.xlabel("t")
    plt.ylabel("radians")

    plt.subplot(414)
    plt.title("Thrust Input")
    plt.plot(t, x[4], label="az")
    plt.legend(loc=0)
    plt.xlabel("t")
    plt.ylabel("acceleration")

    plt.show()


def plot_inputs_based_trajectory(T, dt, n, m, x0, result):
    # plot real trajectory using only inputs
    x = [x0[0][0], x0[0][1], x0[0][2], x0[0][3]]
    pos_x = [x[0]]
    pos_y = [x[2]]
    states = result.x.reshape(T, n + m).T
    for i in range(T):
        u = [states[4][i], states[5][i]]
        x = quadrotor(x, u, dt)
        pos_x.append(x[0])
        pos_y.append(x[2])

    plt.title("Optimal Control Trajectory")
    plt.plot(pos_x, pos_y)
    plt.xlabel("x")
    plt.ylabel("z")
    plt.show()


def cost_func(x, args):
    cost = 0.0

    # convert the (N * T) vector to T x N matrix
    states = x.reshape(args["T"], args["n"] + args["m"]).T
    traj = args["traj"].T

    # position error cost
    cost += 0.0 * np.linalg.norm(states[0] - traj[0])  # dx
    cost += 0.0 * np.linalg.norm(states[2] - traj[1])  # dz

    # control input cost
    cost += 0.5 * np.linalg.norm(states[4])  # az
    cost += 1.0 * np.linalg.norm(states[5])  # theta

    # control input difference cost
    cost += 0.0 * np.linalg.norm(np.diff(states[4]))  # az
    cost += 1.0 * np.linalg.norm(np.diff(states[5]))  # theta

    # end cost to bias to matched landing ??
    # cost += 15.0 * pow(x[-1], 2)

    return cost


def ine_constraints(x, *args):
    T = args[0]
    dt = args[1]
    n = args[2]
    m = args[3]

    # setup
    retval = np.array([])
    state_curr = np.array([])
    state_feas = np.array([])

    # convert the (N * T) vector to T x N matrix
    x = x.reshape(T, n + m)

    # calculate vector valued inequality constraints
    x_prev = x[0]
    for x_curr in x[1:]:
        c_dx = 0.5
        c_dz = 0.2
        g = 10.0

        h_ge = 0.5
        k_ge = 0.2
        a_ge = k_ge * (h_ge - min(h_ge, x_curr[2])) / h_ge

        # calculate error between optimized velocity and feasible velocity
        state_curr = np.array(x_curr[0:4])
        state_feas = x_prev[0:4] + np.array([
            x_prev[1],
            x_prev[4] * sin(x_prev[5]) - c_dx * x_prev[1],
            x_prev[3],
            (x_prev[4] * cos(x_prev[5])) * (1 + a_ge) - g - c_dz * x_prev[3]
        ]) * dt

        retval = np.append(retval, state_feas - state_curr)
        x_prev = x_curr

    return retval


def optimize(p0_z, pf_z, v, dt, vz_max=-1.0):
    n = 4       # num of states
    m = 2       # num of inputs

    x0, traj, T = desired_system(p0_z, pf_z, v, dt)

    # state bounds
    state_bounds = (
        (None, None),  # x
        (None, None),  # vx
        (0, None),     # z
        (vz_max, None)   # vz
    )

    # input bounds
    input_bounds = (
        (0, 20),  # az
        (-0.5235, 0.5235)  # theta
    )

    # generate bounds for all time steps
    bounds = generate_bounds(T, n, m, state_bounds, input_bounds)

    # constraints
    args = {"traj": traj, "T": T, "n": n, "m": m}
    constraints = [
        # equality constraint for start position
        {"type": "eq", "fun": lambda x: np.array([x[0] - x0[0][0]])},  # x
        {"type": "eq", "fun": lambda x: np.array([x[1] - x0[0][1]])},  # vx
        {"type": "eq", "fun": lambda x: np.array([x[2] - x0[0][2]])},  # z
        {"type": "eq", "fun": lambda x: np.array([x[3] - x0[0][3]])},  # vz
        {"type": "eq", "fun": lambda x: np.array([x[4] - x0[0][4]])},  # az
        {"type": "eq", "fun": lambda x: np.array([x[5] - x0[0][5]])},  # theta

        # equality constraint for end position
        {"type": "eq", "fun": lambda x: np.array([x[-6] - x0[-1][0]])},  # x
        {"type": "eq", "fun": lambda x: np.array([x[-5] - 0.0])},        # vx
        {"type": "eq", "fun": lambda x: np.array([x[-4] - x0[-1][1]])},  # z
        {"type": "eq", "fun": lambda x: np.array([x[-3] - 0.0])},        # vz
        {"type": "eq", "fun": lambda x: np.array([x[-2] - 0.0])},        # az
        {"type": "eq", "fun": lambda x: np.array([x[-1] - 0.0])},        # theta

        # nonlinear equality constraint for motion
        {"type": "eq", "fun": ine_constraints, "args": (T, dt, n, m)}
    ]

    # optimize
    results = scipy.optimize.minimize(cost_func,
                                      x0,
                                      args=args,
                                      constraints=constraints,
                                      bounds=bounds)

    # plot optimization results
    # plot_optimization_results(traj, results.x, T, n, m)
    # plot_inputs_based_trajectory(T, dt, n, m, x0, results)

    return (T, n, m, v, dt, results)


def record_optimized_results(T, n, m, v, dt, results, fpath):
    # setup
    f = open(fpath, "wb")
    header = "x,vx,z,vz,az,theta,tbf_x,tbf_z\n"
    f.write(bytes(header, "UTF-8"))

    target_x = 0.0
    states = results.x.reshape(T, n + m)

    for x in states:
        target_x += v * dt
        tbf_x = target_x - x[0]
        tbf_z = 0.0 - x[2]

        f.write(bytes(str(x[0]) + ",", "UTF-8"))   # x
        f.write(bytes(str(x[1]) + ",", "UTF-8"))   # vx
        f.write(bytes(str(x[2]) + ",", "UTF-8"))   # z
        f.write(bytes(str(x[3]) + ",", "UTF-8"))   # vz
        f.write(bytes(str(x[4]) + ",", "UTF-8"))   # az
        f.write(bytes(str(x[5]) + ",", "UTF-8"))   # theta
        f.write(bytes(str(tbf_x) + ",", "UTF-8"))  # tbf_x
        f.write(bytes(str(tbf_z), "UTF-8"))        # tbf_z
        f.write(bytes("\n", "UTF-8"))

    f.close()


def generate_trajectory_combinations():
    p0_z = frange(5, 6, 1, 1)
    pf_z = [0.0]
    v = frange(0, 5, 1)

    conditions = [p0_z, pf_z, v]
    conditions = list(itertools.product(*conditions))
    return conditions


if __name__ == "__main__":
    basedir = "./trajectory/"
    index = 0
    dt = 0.1

    # p0_z = 5.0
    # pf_z = 0.0
    # v = 0.0
    # T, n, m, v, dt, results = optimize(p0_z, pf_z, v, dt)
    # filepath = basedir + "0.csv"
    # record_optimized_results(T, n, m, v, dt, results, filepath)

    # prep index file
    index_file = open(basedir + "index.csv", "wb")
    index_file.write(bytes("index,p0_z,v\n", "UTF-8"))

    # create trajectory table
    combinations = generate_trajectory_combinations()
    for c in combinations:
        p0_z = c[0]
        pf_z = 0.0
        v = c[2]
        filepath = basedir + str(index) + ".csv"
        index_line = "{0},{1},{2}\n".format(index, p0_z, v)
        index_file.write(bytes(index_line, "UTF-8"))
        index += 1

        print("Optimizing starting height {0} @ {1}ms^-1".format(p0_z, v))
        T, n, m, v, dt, results = optimize(p0_z, pf_z, v, dt)
        record_optimized_results(T, n, m, v, dt, results, filepath)

    # close index file
    index_file.close()
