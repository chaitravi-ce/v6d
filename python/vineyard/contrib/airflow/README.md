Apache Airflow Provider for Vineyard
====================================

The apache airflow provider for vineyard contains components to sharing intermediate
data among tasks in Airflow workflows using Vineyard.

Vineyard works as a *XCom* backend for airflow workers to allow transferring
large-scale data objects between tasks that cannot be fit into the Airflow's
database backend without involving external storage systems like HDFS. The
Vineyard XCom backend handles object migration as well when the required inputs
is not located on where the task is scheduled to execute.

Requirements
------------

The following packages are needed to run Airflow on Vineyard,

- airflow >= 2.1.0
- vineyard >= 0.2.12

Configuration and Usage
-----------------------

1. Install required packages:

        pip3 install airflow-provider-vineyard

2. Configure Vineyard locally

    The vineyard server can be easier launched locally with the following command:

        vineyardd --socket=/tmp/vineyard.sock

    See also our documentation about [launching vineyard][1].

3. Configure Airflow to use the vineyard XCom backend by specifying the environment
    variable

        export AIRFLOW__CORE__XCOM_BACKEND=vineyard.contrib.airflow.xcom.VineyardXCom

    and configure the location of UNIX-domain IPC socket for vineyard client by

        export AIRFLOW__VINEYARD__IPC_SOCKET=/tmp/vineyard.sock

    or

        export VINEYARD_IPC_SOCKET=/tmp/vineyard.sock

4. Launching your airflow scheduler and workers, and run the following DAG as example,

        ```python
        import numpy as np
        import pandas as pd

        from airflow.decorators import dag, task
        from airflow.utils.dates import days_ago

        default_args = {
            'owner': 'airflow',
        }

        @dag(default_args=default_args, schedule_interval=None, start_date=days_ago(2), tags=['example'])
        def taskflow_etl_pandas():
            @task()
            def extract():
                order_data_dict = pd.DataFrame({
                    'a': np.random.rand(100000),
                    'b': np.random.rand(100000)
                })
                return order_data_dict

            @task(multiple_outputs=True)
            def transform(order_data_dict: dict):
                return {"total_order_value": order_data_dict["a"].sum()}

            @task()
            def load(total_order_value: float):
                print(f"Total order value is: {total_order_value:.2f}")

            order_data = extract()
            order_summary = transform(order_data)
            load(order_summary["total_order_value"])

        taskflow_etl_pandas_dag = taskflow_etl_pandas()
        ```

In above example, task :code:`extract` and task :code:`transform` shares a
:code:`pandas.DataFrame` as the intermediate data, which is impossible as
it cannot be pickled and when the data is large, it cannot be fit into the
table in backend databases of Airflow.

The example is adapted from the documentation of Airflow, see also
[Tutorial on the Taskflow API][2].

Run the tests
-------------

1. Start your vineyardd with the following command,

        python3 -m vineyard

2. Set airflow to use the vineyard XCom backend, and run tests with pytest,

        export AIRFLOW__CORE__XCOM_BACKEND=vineyard.contrib.airflow.xcom.VineyardXCom

        pytest -s -vvv python/vineyard/contrib/airflow/tests/test_python_dag.py
        pytest -s -vvv python/vineyard/contrib/airflow/tests/test_pandas_dag.py


The pandas test suite is not possible to run with the default XCom backend, vineyard
enables airflow to exchange **complex** and **big** data without modify the DAG and tasks!

[1]: https://v6d.io/notes/getting-started.html#starting-vineyard-server
[2]: https://airflow.apache.org/docs/apache-airflow/stable/tutorial_taskflow_api.html
