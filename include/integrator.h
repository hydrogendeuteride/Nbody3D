#ifndef NBODY3D_INTEGRATOR_H
#define NBODY3D_INTEGRATOR_H

template<typename Acc>
class Velocity_Verlet
{
private:
    float dampingFactor = 1.0f;

public:

    void setDampingFactor(float damping)
    {
        dampingFactor = damping;
    }

    void operator()(Acc acc, const float dt, const SimulationData &data)
    {
#pragma omp parallel for
        for (int i = 0; i < MAX_PARTICLES; ++i)
        {
            unsigned int particleIndex = data.idxSorted[i];
            acc(particleIndex, data);
        }
#pragma omp parallel for
        for (int i = 0; i < MAX_PARTICLES; ++i)
        {
            unsigned int particleIndex = data.idxSorted[i];

            data.particleX[i] += data.particleVelX[particleIndex] * dt + 0.5f * data.accX[particleIndex] * dt * dt;
            data.particleY[i] += data.particleVelY[particleIndex] * dt + 0.5f * data.accY[particleIndex] * dt * dt;
            data.particleZ[i] += data.particleVelZ[particleIndex] * dt + 0.5f * data.accZ[particleIndex] * dt * dt;
        }

#pragma omp parallel for
        for (int i = 0; i < MAX_PARTICLES; ++i)
        {
            unsigned int particleIndex = data.idxSorted[i];
            float acc_oldX = data.accX[particleIndex];
            float acc_oldY = data.accY[particleIndex];
            float acc_oldZ = data.accZ[particleIndex];

            acc(particleIndex, data);

            data.particleVelX[particleIndex] += 0.5f * (acc_oldX + data.accX[particleIndex]) * dt * dampingFactor;
            data.particleVelY[particleIndex] += 0.5f * (acc_oldY + data.accY[particleIndex]) * dt * dampingFactor;
            data.particleVelZ[particleIndex] += 0.5f * (acc_oldZ + data.accZ[particleIndex]) * dt * dampingFactor;
        }
    }
};

#endif //NBODY3D_INTEGRATOR_H
