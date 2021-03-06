/**
 * @file perceptron_impl.hpp
 * @author Udit Saxena
 *
 * Implementation of Perceptron class.
 */
#ifndef __MLPACK_METHODS_PERCEPTRON_PERCEPTRON_IMPL_HPP
#define __MLPACK_METHODS_PERCEPTRON_PERCEPTRON_IMPL_HPP

#include "perceptron.hpp"

namespace mlpack {
namespace perceptron {

/**
 * Constructor - constructs the perceptron. Or rather, builds the weights
 * matrix, which is later used in classification.  It adds a bias input vector
 * of 1 to the input data to take care of the bias weights.
 *
 * @param data Input, training data.
 * @param labels Labels of dataset.
 * @param iterations Maximum number of iterations for the perceptron learning
 *      algorithm.
 */
template<
    typename LearnPolicy,
    typename WeightInitializationPolicy,
    typename MatType
>
Perceptron<LearnPolicy, WeightInitializationPolicy, MatType>::Perceptron(
    const MatType& data,
    const arma::Row<size_t>& labels,
    const int iterations)
{
  WeightInitializationPolicy WIP;
  WIP.Initialize(weights, biases, data.n_rows, arma::max(labels) + 1);

  // Start training.
  iter = iterations;
  Train(data, labels);
}


/**
 * Classification function. After training, use the weights matrix to classify
 * test, and put the predicted classes in predictedLabels.
 *
 * @param test testing data or data to classify.
 * @param predictedLabels vector to store the predicted classes after
 *      classifying test
 */
template<
    typename LearnPolicy,
    typename WeightInitializationPolicy,
    typename MatType
>
void Perceptron<LearnPolicy, WeightInitializationPolicy, MatType>::Classify(
    const MatType& test,
    arma::Row<size_t>& predictedLabels)
{
  arma::vec tempLabelMat;
  arma::uword maxIndex;

  // Could probably be faster if done in batch.
  for (size_t i = 0; i < test.n_cols; i++)
  {
    tempLabelMat = weights.t() * test.col(i) + biases;
    tempLabelMat.max(maxIndex);
    predictedLabels(0, i) = maxIndex;
  }
}

/**
 * Alternate constructor which copies parameters from an already initiated
 * perceptron.
 *
 * @param other The other initiated Perceptron object from which we copy the
 *     values from.
 * @param data The data on which to train this Perceptron object on.
 * @param D Weight vector to use while training. For boosting purposes.
 * @param labels The labels of data.
 */
template<
    typename LearnPolicy,
    typename WeightInitializationPolicy,
    typename MatType
>
Perceptron<LearnPolicy, WeightInitializationPolicy, MatType>::Perceptron(
    const Perceptron<>& other,
    const MatType& data,
    const arma::rowvec& D,
    const arma::Row<size_t>& labels)
{
  iter = other.iter;

  // Insert a row of ones at the top of the training data set.
  WeightInitializationPolicy WIP;
  WIP.Initialize(weights, biases, data.n_rows, arma::max(labels) + 1);

  Train(data, labels, D);
}

//! Serialize the perceptron.
template<typename LearnPolicy,
         typename WeightInitializationPolicy,
         typename MatType>
template<typename Archive>
void Perceptron<LearnPolicy, WeightInitializationPolicy, MatType>::Serialize(
    Archive& ar,
    const unsigned int /* version */)
{
  // For now, do nothing.
}

/**
 * Training Function. It trains on trainData using the cost matrix D
 *
 * @param D Cost matrix. Stores the cost of mispredicting instances
 */
template<
    typename LearnPolicy,
    typename WeightInitializationPolicy,
    typename MatType
>
void Perceptron<LearnPolicy, WeightInitializationPolicy, MatType>::Train(
    const MatType& data,
    const arma::Row<size_t>& labels,
    const arma::rowvec& D)
{
  size_t j, i = 0;
  bool converged = false;
  size_t tempLabel;
  arma::uword maxIndexRow, maxIndexCol;
  arma::mat tempLabelMat;

  LearnPolicy LP;

  const bool hasWeights = (D.n_elem > 0);

  while ((i < iter) && (!converged))
  {
    // This outer loop is for each iteration, and we use the 'converged'
    // variable for noting whether or not convergence has been reached.
    i++;
    converged = true;

    // Now this inner loop is for going through the dataset in each iteration.
    for (j = 0; j < data.n_cols; j++)
    {
      // Multiply for each variable and check whether the current weight vector
      // correctly classifies this.
      tempLabelMat = weights.t() * data.col(j) + biases;

      tempLabelMat.max(maxIndexRow, maxIndexCol);

      // Check whether prediction is correct.
      if (maxIndexRow != labels(0, j))
      {
        // Due to incorrect prediction, convergence set to false.
        converged = false;
        tempLabel = labels(0, j);

        // Send maxIndexRow for knowing which weight to update, send j to know
        // the value of the vector to update it with.  Send tempLabel to know
        // the correct class.
        if (hasWeights)
          LP.UpdateWeights(data.col(j), weights, biases, maxIndexRow, tempLabel,
              D(j));
        else
          LP.UpdateWeights(data.col(j), weights, biases, maxIndexRow,
              tempLabel);
      }
    }
  }
}

} // namespace perceptron
} // namespace mlpack

#endif
