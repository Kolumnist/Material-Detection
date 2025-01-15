FROM continuumio/miniconda3:latest

# Set up a working directory inside the container
# This will be where all your code and data will reside.
WORKDIR /app

# Copy your project files into the container
# The "." refers to the current directory on the host, and "/app" is the target in the container.
COPY . /app

# Install necessary Python dependencies
# Use Conda to create an environment and install dependencies.
# Assuming you have a requirements.txt file for dependencies.
RUN conda install --yes python=3.10

RUN pip freeze > requirements.txt
    
# RUN conda install --yes --file requirements.txt

RUN conda clean --all -f -y

# Install pytorch and stuff
RUN conda install pytorch torchvision torchaudio cpuonly -c pytorch
RUN conda install psycopg2

# Dependencies for labelstudio
RUN pip install fsspec
RUN pip install sympy==1.13.1
RUN pip install label-studio

# Dependencies for kaggle dataset
# conda install -n detection ipykernel --update-deps --force-reinstall


# Add environment variables to support Miniconda
# These ensure Conda works properly inside the container.
ENV PATH /opt/conda/bin:$PATH
ENV PYTHONUNBUFFERED 1

# Expose any necessary ports (optional)
# If your Python application runs a web server or API, expose that port.
EXPOSE 5000

# Set the default command to run your Python script
# Replace `train.py` with the main Python script for your project.
CMD ["python", "hello.py"]