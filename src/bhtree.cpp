#include <stack>
#include "bhtree.h"
#include "omp.h"

bool isParticleInNode(float pX, float pY, float nX, float nY, float nW, float nH)
{
    return pX >= nX && pX <= nX + nW &&
           pY >= nY && pY <= nY + nH;
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
        float dist = std::sqrt(distX * distX + distY * distY);

        // Check if the current node is sufficiently far away or a leaf node
        if (data.nodeWidth[top] / dist <= THETA || data.nodeChildren[top][0] == NULL_INDEX)
        {
            // Ensure the particle is not in the current node
            if (!isParticleInNode(data.particleX[particleIdx], data.particleY[particleIdx], data.nodeX[top],
                                  data.nodeY[top],
                                  data.nodeWidth[top], data.nodeHeight[top]))
            {
                // Add gravitational acceleration from the current node to the particle
                data.accX[particleIdx] += gravity(data.nodeTotalMass[top], distX, distY).x;
                data.accY[particleIdx] += gravity(data.nodeTotalMass[top], distX, distY).y;
            }
        }
        else
        {
            // If the node is not sufficiently far away, add its children to the stack for further examination
            for (int i = 0; i < 4; ++i)
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
}

void updateAllParticles(float damping, float dt, const SimulationData &data)
{
    Velocity_Verlet<decltype(&netAcceleration)> integrator;
    integrator.setDampingFactor(damping);
    integrator(netAcceleration, dt, data);

#pragma omp parallel for
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        boundaryDetection(i, 1.0f, data);
    }
}