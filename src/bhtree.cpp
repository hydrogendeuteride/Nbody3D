#include <stack>
#include "bhtree.h"
#include "omp.h"

bool isParticleInNode(float pX, float pY, float pZ, float nX, float nY, float nZ, float nW, float nH, float nD)
{
    return pX >= nX && pX <= nX + nW &&
           pY >= nY && pY <= nY + nH &&
           pZ >= nZ && pZ <= nZ + nD;
}

void netAcceleration(int particleIdx, const SimulationData &data)
{
    std::stack<int> stack;
    stack.push(0);

    Gravitational gravity;

    while (!stack.empty())
    {
        auto top = stack.top();
        stack.pop();

        float distX = data.particleX[particleIdx] - data.nodeCOM_X[top];
        float distY = data.particleY[particleIdx] - data.nodeCOM_Y[top];
        float distZ = data.particleZ[particleIdx] - data.nodeCOM_Z[top];
        float dist = std::sqrt(distX * distX + distY * distY + distZ * distZ);

        // Check if the current node is sufficiently far away or a leaf node
        if (data.nodeWidth[top] / dist <= THETA || data.nodeChildren[top][0] == NULL_INDEX)
        {
            // Ensure the particle is not in the current node
            if (!isParticleInNode(data.particleX[particleIdx], data.particleY[particleIdx], data.particleZ[particleIdx],
                                  data.nodeX[top], data.nodeY[top], data.nodeZ[top],
                                  data.nodeWidth[top], data.nodeHeight[top], data.nodeDepth[top]))
            {
                // Add gravitational acceleration from the current node to the particle
                vec tmp = gravity(data.nodeTotalMass[top], distX, distY, distZ);
                data.accX[particleIdx] += tmp.x;
                data.accY[particleIdx] += tmp.y;
                data.accY[particleIdx] += tmp.z;
            }
        }
        else
        {
            // If the node is not sufficiently far away, add its children to the stack for further examination
            for (int i = 0; i < OCT_CHILD; ++i)
            {
                if (data.nodeChildren[top][i] != NULL_INDEX)
                    stack.push(data.nodeChildren[top][i]);
            }
        }
    }
}

void boundaryDetection(int particleIdx, float offset, const SimulationData &data)
{
    if (data.particleVelX[particleIdx] > 0 && data.particleX[particleIdx] > data.nodeWidth[0])
    {
        data.particleVelX[particleIdx] = -data.particleVelX[particleIdx];
        data.particleX[particleIdx] = data.nodeWidth[0] - data.nodeX[0] - offset;
    }
    else if (data.particleVelX[particleIdx] < 0 && data.particleX[particleIdx] < 0)
    {
        data.particleVelX[particleIdx] = -data.particleVelX[particleIdx];
        data.particleX[particleIdx] = data.nodeX[0] + offset;
    }

    if (data.particleVelY[particleIdx] > 0 && data.particleY[particleIdx] > data.nodeHeight[0])
    {
        data.particleVelY[particleIdx] = -data.particleVelY[particleIdx];
        data.particleY[particleIdx] = data.nodeHeight[0] - data.nodeY[0] - offset;
    }
    else if (data.particleVelY[particleIdx] < 0 && data.particleY[particleIdx] < 0)
    {
        data.particleVelY[particleIdx] = -data.particleVelY[particleIdx];
        data.particleY[particleIdx] = data.nodeY[0] + offset;
    }

    if (data.particleVelZ[particleIdx] > 0 && data.particleZ[particleIdx] > data.nodeDepth[0])
    {
        data.particleVelZ[particleIdx] = -data.particleVelZ[particleIdx];
        data.particleZ[particleIdx] = data.nodeDepth[0] - data.nodeZ[0] - offset;
    }
    else if (data.particleVelZ[particleIdx] < 0 && data.particleZ[particleIdx] < 0)
    {
        data.particleVelZ[particleIdx] = -data.particleVelZ[particleIdx];
        data.particleZ[particleIdx] = data.nodeZ[0] + offset;
    }
}

void updateAllParticles(float damping, float dt, const SimulationData &data)
{
    Velocity_Verlet<decltype(&netAcceleration)> (netAcceleration, damping, dt, data);

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        //boundaryDetection(i, 1.0f, data);
    }
}