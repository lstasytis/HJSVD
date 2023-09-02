
import numpy as np
from itertools import count
import itertools


def sqrt_block(row):
    norm = 0
    for i in range(len(row)):
        norm += row[i] * row[i]
    norm = np.sqrt(norm)
    return norm


def gain_factor(iterations=32):
    gain = 1
    for i in range(iterations):
        gain *= np.cos(np.arctan(2**-i))

    return gain



def cov_block(col_a,col_b):
    # input is nx1 size col_a and nx1 size col_b. 
    # outputs are all scalar
    # 3n multiplier routine


    alpha,beta,cov_new = 0.0, 0.0, 0.0
    for i in range(col_a.size):
        alpha           += col_a[i] * col_a[i] # A.T @ A
        beta            += col_b[i] * col_b[i]
        cov_new         += col_a[i] * col_b[i]

    return alpha, beta, cov_new


def rotator_block_cordic_theta_inverse(alpha,beta,gamma,ep):
    # inputs are all scalar
    # outputs are all scalar
    # cordic rotation mode routine

    if abs(gamma)/np.sqrt(alpha*beta) <= ep:
        theta = 0.0
    else:
        eta = (beta-alpha)/(2.0*gamma)                      # CORDIC vector-circular y=beta-alpha, x = 2.0 * gamma(leftshift 1)?
        eta2 = np.sqrt(1+eta**2)                            # CORDIC vector-circular x=1,y=eta
        theta = np.arctan(np.sign(eta)/(abs(eta)+eta2) )    # CORDIC vector-circular z=0, x = sign(eta), y=abs(eta)+eta2

        # theta inner product is 't' in most literature

    return theta


def rotator_block_cordic(alpha,beta,gamma):
    # inputs are all scalar
    # outputs are all scalar
    # cordic rotation mode routine

    eta = (alpha-beta)/(2*gamma)    # CORDIC vector mode

    theta = (1/2) * np.arctan(eta)  # CORDIC vector mode

    return theta


def rotator_block_multiplier(alpha,beta,gamma,ep):
    # inputs are all scalar
    # outputs are all scalar

    if abs(gamma)/np.sqrt(alpha*beta) <= ep:
        c = 1.0
        s = 0.0
    else:
        eta = (beta-alpha)/(2.0*gamma)                      # multiplier routine
        t = np.sign(eta)/(abs(eta)+np.sqrt(1+eta**2))       # multiplier routine
        c = 1/np.sqrt(1 + t*t)                              #LUT
        s = c*t                                             # multiplier routine

    return c,s

def updater_block_multiplier(col_a,col_b,c,s):
    # inputs: nx1, nx1, scalar,scalar
    # outputs: nx1,nx1
    col_a_copy = col_a.copy()
    
    for i in range(col_a.size):
        col_a[i] = c*col_a_copy[i] - s*col_b[i] # 1 CORDIC or 2 mults OR 1 TOTAL? CHECK
        col_b[i] = s*col_a_copy[i] + c*col_b[i] # 1 CORDIC or 2 mults

    return col_a,col_b


def updater_block_cordic(col_a,col_b,theta):
    # inputs: nx1, nx1, scalar,scalar
    # outputs: nx1,nx1
    gain = 1/gain_factor()
    #gain = 1.0
    

    col_a_copy = col_a.copy()

    for i in range(col_a.size):
        #cordic version
        col_a[i] = gain*(np.cos(theta)*col_a_copy[i] - np.sin(theta)*col_b[i]) # CORDIC rotation-circular mode
        col_b[i] = gain*(np.sin(theta)*col_a_copy[i] + np.cos(theta)*col_b[i]) # CORDIC rotation-circular mode


    return col_a,col_b


def updater_block_cordic_inv(col_a,col_b,c,s):
    # inputs: nx1, nx1, scalar,scalar
    # outputs: nx1,nx1
    theta = np.arcsin(s)

    col_a_copy = col_a.copy()

    for i in range(col_a.size):
        #cordic version
        col_a[i] = np.cos(theta)*col_a_copy[i] - np.sin(theta)*col_b[i] # CORDIC rotation-circular mode
        col_b[i] = np.sin(theta)*col_a_copy[i] + np.cos(theta)*col_b[i] # CORDIC rotation-circular mode

    return col_a,col_b



def PU_Multi(U_i,U_j,V_i,V_j,cov,ep):
    #print(f"pulling cols: {i,j}")
    alpha,beta,cov_new = cov_block(U_i,U_j) # 3n multipliers
    cov = max(cov,abs(cov_new)/np.sqrt(alpha*beta))

    c,s = rotator_block_multiplier(alpha,beta,cov_new,ep) # multipliers and dividers

    print(f"s,c: {s,c}")

    U_i,U_j = updater_block_multiplier(U_i,U_j,c,s) # 4 multis
    V_i,V_j = updater_block_multiplier(V_i,V_j,c,s) # 4 multis

    return U_i,U_j,V_i,V_j,cov

def PU_CORDIC(U_i,U_j,V_i,V_j,cov,ep):


    alpha,beta,cov_new = cov_block(U_i,U_j) # 3n multipliers

    cov = max(cov,abs(cov_new)/np.sqrt(alpha*beta)) # Comparator block
    theta = rotator_block_cordic_theta_inverse(alpha,beta,cov_new,ep)

    U_i,U_j = updater_block_cordic(U_i,U_j,theta) # 1 CORDIC in rota mode
    V_i,V_j = updater_block_cordic(V_i,V_j,theta) # 1 CORDIC in rota mode

    return U_i,U_j,V_i,V_j,cov



def rr_tournament(n):
    teams = list(range(0,n))    
    n = len(teams)
    matchs = []
    fixtures = []
    return_matchs = []
    for fixture in range(1,n):
        for i in range(int(n/2)):
            matchs.append((teams[i], teams[n - 1 - i]))
            return_matchs.append((teams[n - 1 - i], teams[i]))
        teams.insert(1, teams.pop())
        fixtures.insert(int(len(fixtures)/2), matchs)
        fixtures.append(return_matchs)
        matchs = []
        return_matchs = []
    return fixtures



def float_to_fixed(a):
    width = 32 
    return np.int32(a * (1 << width-1))

def fixed_to_float(a):
    width = 32
    return np.int32(a)  / (1 << width-1)



def HJSVD(A,mode=0,tol=1.e-10):

    tolerance = tol
    U = A.copy()
    n = A.shape[1] # col length
    V = np.identity(n)
    cov = tolerance+1
    players = range(1,n+1)
    pairs = list(itertools.combinations(players,2))
    gain = gain_factor()
    print(f'gain factor: {1/gain} compensation: {gain} ')
    ite=0

    rr = rr_tournament(n)

    rr_len = int(len(rr)/2)
    rr = rr[:rr_len]

    #n = 2
    while cov > tolerance:
        ite+=1
        #print(ite,cov)
        cov = 0
        if mode == 2:
            for i in range(0, n-1):
                for j in range(i+1,n):
                    # Computing alpha,beta,gamma
                    alpha = np.dot(U[:, i].T, U[:, i])
                    beta = np.dot(U[:, j].T, U[:, j])
                    # print(beta)
                    gamma = np.dot(U[:, j].T, U[:, i])
                    # print(gamma)
                    cov = max(cov, abs(gamma)/np.sqrt(alpha*beta))
                    # print(converge)
                    if abs(gamma)/np.sqrt(alpha*beta) <= tolerance:
                        c = 1.0
                        s = 0.0
                    else:
                        eta = (beta-alpha)/(2.0*gamma)
                        # print(eta)
                        t = np.sign(eta)/(abs(eta)+np.sqrt(1+eta**2))
                        # print(t)
                        c = 1/np.sqrt(1 + t*t)
                        # print(c)
                        s = c*t
                    # print(s)
                    # Updating the columns i and j of U
                    t = U[:, i].copy()
                    U[:, i] = c*t - s*U[:, j]
                    U[:, j] = s*t + c*U[:, j]
                    # Updating the columns i and j of V
                    t = V[:, i].copy()
                    V[:, i] = c*t - s*V[:, j]
                    V[:, j] = s*t + c*V[:, j]

        else:

            round_count = 0
            for round in rr:
                for pair in round:
                    i,j = pair
                    if mode == 0: # multi
                        U[:, i], U[:, j], V[:, i], V[:, j],cov = PU_Multi(U[:, i], U[:, j], V[:, i], V[:, j],cov,tolerance)
                    else: # cordic
                        U[:, i], U[:, j], V[:, i], V[:, j],cov = PU_CORDIC(U[:, i], U[:, j], V[:, i], V[:, j],cov,tolerance)   

                if mode == 1: # cordic

                    U *= gain                   
                    V *= gain
                round_count+=1


    sigma = np.zeros(n)
    #print(U)
    for i in range(n):
        norm = sqrt_block(U[:,i])
        sigma[i] = norm
        U[:, i] = U[:, i]/norm

    return [U, sigma, V.T]


def sort_components(U,S,V):
    indx = (np.argsort(S))[::-1]
    S  = S[indx]
    U = U[:,indx]
    V = V[indx,:]
    return U,S,V

